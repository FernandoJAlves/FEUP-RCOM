#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "macros.h"
#include "stateMachine.h"
#include "data_link.h"
#include "application_layer.h"
#include "writer.h"
#include "reader.h"

unsigned int writer_msg_count = 0;

int main(int argc, char **argv)
{

  if ((argc < 4 && ((strcmp("S", argv[2]) == 0))) || (argc < 3 && ((strcmp("R", argv[2]) == 0))) ||
      ((strcmp("1", argv[1]) != 0) && (strcmp("2", argv[1]) != 0)) ||
      ((strcmp("S", argv[2]) != 0) && (strcmp("R", argv[2]) != 0)))
  {

    printf("Usage:\tnserial SerialPort ComunicationMode\n\tex: nserial [1|2] [S|R] [fileName]\n");
    exit(1);
  }

  if (!strcmp(argv[1], "1"))
  {
    link_layer.port = COM_1;
  }
  else if (!strcmp(argv[1], "2"))
  {
    link_layer.port = COM_2;
  }
  else
  {
    perror("porta 1 ou 2 \n");
    return -1;
  }
  (void)signal(SIGALRM, timeout);
  if (!strcmp(argv[2], "S"))
  {
    link_layer.status = TRANSMITTER;
    data_writer(argc, argv);
  }
  else if (!strcmp(argv[2], "R"))
  {
    link_layer.status = RECEIVER;
    data_reader(argc, argv);
  }
  else
  {
    perror("S or R\n");
    return -1;
  }

  return 0;
}

void data_writer(int argc, char *argv[])
{

  int fd;

  fd = llopenW(1, 2);

  // TODO - Tirar hardcoded
  int file_name_size = strlen(argv[3]);
  char *file_name = argv[3];

  off_t fileSize;
  long int controlPacketSize = 0;
  unsigned char *file = readFile(file_name, &fileSize);
  printf("fileSize: %ld\n", fileSize);
  unsigned char *pointerToCtrlPacket = makeControlPackage_I(fileSize, file_name, file_name_size, &controlPacketSize, CTRL_C_START);



  llwriteW(fd, pointerToCtrlPacket, controlPacketSize);
  int packetSize = PACKET_SIZE;
  long int curr_index = 0;
  unsigned long progress = 0;

  while (curr_index < fileSize && packetSize == PACKET_SIZE)
  {

    //get a piece of the file, then add the header, then send
    unsigned char *packet = splitFile(file, &curr_index, &packetSize, fileSize);

    int packetHeaderSize = packetSize;
    unsigned char *packet_and_header = makePacketHeader(packet, fileSize, &packetHeaderSize);

    //TODO - Testar se o llwriteW falhou
    
    progress = (unsigned long)(((double)curr_index/(double)fileSize)*100);
    printf("\n===============\n");
    printf("Progress: %lu%%\n", progress);
    
    llwriteW(fd, packet_and_header, packetHeaderSize);

    printf("Packet enviado: %d\n", writer_msg_count);
  }

  unsigned char *pointerToCtrlPacketEnd = makeControlPackage_I(fileSize, file_name, file_name_size, &controlPacketSize, CTRL_C_END);
  printf("Before the end\n");

  llwriteW(fd, pointerToCtrlPacketEnd, controlPacketSize);
  printf("Control Packet END sent\n %x", pointerToCtrlPacket[0]);
  //free(pointerToCtrlPacket);
  //free(pointerToCtrlPacketEnd);
  //free(file);
  llcloseW(fd);
}

void data_reader(int argc, char *argv[])
{
  int reading = 1;
  int fd = llopenR(1, 2);
  unsigned long size = 0;
  expectedBCC = 0;
  
  char * fileName = (char*)malloc(0);
  int fileSizeBytes = 0;
  int fileNameSize = 0;
  unsigned long totalSize = 0;
  
  
  unsigned char *startPacket = llread(fd, &size);
  getStartPacketData(startPacket,&totalSize,&fileSizeBytes,&fileNameSize,fileName);
  
  
  
  free(startPacket);
  unsigned char *dataPacket;
  unsigned char *finalFile = malloc(0);
  off_t index = 0;
  unsigned long fileSize = 0;

 


  while (reading)
  {
    printf("\n================\n");
    dataPacket = llread(fd, &size);
    fileSize += size;
    if (size == 0)
    {
      continue;
    }
    if (dataPacket[0] == CTRL_C_END)
    {
      reading = 0;
      break;
    }
    //remove headers
    dataPacket = removeHeaders(dataPacket, &size);
    finalFile = (unsigned char *)realloc(finalFile,fileSize);
    memcpy(finalFile + index, dataPacket, size);
    index += size;
    free(dataPacket);
  }
  printf("value of size:  %lu", index);
  //fileName[0] = 'c';
  createFile(finalFile, &index, fileName);
  free(finalFile);
  free(fileName);
  llcloseR(fd);
}

void createFile(unsigned char *mensagem, off_t *sizeFile, char *filename)
{
  FILE *file = fopen(filename, "wb+");
  fwrite((void *)mensagem, 1, *sizeFile, file);
  printf("%zd\n", *sizeFile);
  printf("New file created\n");
  fclose(file);
}

int receivedEND(unsigned char *start, int sizeStart, unsigned char *end, int sizeEnd)
{
  int s = 1;
  int e = 1;
  if (sizeStart != sizeEnd)
    return 0;
  else
  {
    if (end[0] == CTRL_C_END)
    {
      for (; s < sizeStart; s++, e++)
      {
        if (start[s] != end[e])
          return 0;
      }
      return 1;
    }
    else
    {
      return 0;
    }
  }
}

unsigned char *makeControlPackage_I(off_t fileSize, char *fileName, long int fileName_size, long int *finalSize, unsigned char start_or_end)
{
  /*

TLV (Type, Length, Value)
– T (um octeto) – indica qual o parâmetro (0 – tamanho do ficheiro, 1 – nome do
ficheiro, outros valores – a definir, se necessário)
– L (um octeto) – indica o tamanho em octetos do campo V (valor do parâmetro)
– V (número de octetos indicado em L) – valor do parâmetro
  */

  *finalSize = 5 + sizeof(fileSize) + fileName_size;
  unsigned char *finalPackage = (unsigned char *)malloc(sizeof(unsigned char) * (*finalSize));

  if (start_or_end == CTRL_C_START)
  {
    finalPackage[0] = start_or_end;
  }
  else if (start_or_end == CTRL_C_END)
  {
    finalPackage[0] = start_or_end;
  }
  else
  {
    printf("Invalid value in control packet!\n");
    return NULL;
  }
  finalPackage[1] = T1;               //Tamanho do ficheiro
  finalPackage[2] = sizeof(fileSize); //8
  for(int i = finalPackage[2] -1; i >= 0;i--){
    finalPackage[2+finalPackage[2]-i] = (fileSize >> (i*8)) & 0xFF;
  }
  finalPackage[3+finalPackage[2]] = T2;
  finalPackage[4+finalPackage[2]] = fileName_size;
  int i;
  for (i = 0; i < fileName_size; i++)
  {
    finalPackage[5+finalPackage[2]+i] = fileName[i];
  }

  return finalPackage;
}

unsigned char *readFile(char *fileName, off_t *fileSize)
{

  FILE *fd;
  struct stat data;

  if ((fd = fopen(((const char *)fileName), "rb")) == NULL)
  {
    perror("Error while opening the file");
    return NULL;
  }
  stat(fileName, &data); //get the file metadata

  *fileSize = data.st_size; //gets file size in bytes

  unsigned char *fileData = (unsigned char *)malloc(*fileSize);

  fread(fileData, sizeof(unsigned char), *fileSize, fd);
  if (ferror(fd))
  {
    perror("error writting to file\n");
  }
  fclose(fd);
  return fileData;
}

unsigned char *makePacketHeader(unsigned char *fileFragment, long int fileSize, int *sizeOfFragment)
{

  unsigned char *dataPacket = (unsigned char *)malloc(fileSize + 4);

  dataPacket[0] = PACKET_H_C;
  dataPacket[1] = writer_msg_count % 255;
  dataPacket[2] = (int)fileSize / 256;
  dataPacket[3] = (int)fileSize % 256;
  memcpy(dataPacket + 4, fileFragment, *sizeOfFragment);
  *sizeOfFragment += 4;
  writer_msg_count++;
  //TODO ?
  return dataPacket;
}

unsigned char *splitFile(unsigned char *file, long int *curr_index, int *packetSize, long int fileSize)
{
  unsigned char *packet;

  if (*curr_index + *packetSize > fileSize)
  {
    *packetSize = fileSize - *curr_index; //Returns the number of bytes not sent yet
  }
  packet = (unsigned char *)malloc(*packetSize);

  int i = 0;
  int j = *curr_index;
  while (i < *packetSize)
  {
    packet[i] = file[j];
    i++;
    j++;
  }
  *curr_index = j;
  return packet;
}

unsigned char * removeHeaders(unsigned char *packetWithHeader, unsigned long *sizeWithHeaders)
{
  unsigned int i;
  *sizeWithHeaders -= 4;
  unsigned char *newPacket = (unsigned char *)malloc(*sizeWithHeaders);

  for (i = 0; i < *sizeWithHeaders; i++)
  {
    newPacket[i] = packetWithHeader[i + 4];
  }

  return newPacket;
}


void getStartPacketData(unsigned char * packet,unsigned long * fileSize,int * fileSizeBytes,int * fileNameSize, char * fileName){
  if(!(packet[0] == CTRL_C_START && packet[1] == T1)){
    printf("Invalid Packet: Not start packet\n");
  }
  *fileSizeBytes = (int)packet[2];
  for(int i = 0; i < *fileSizeBytes;i++){
    *fileSize = (*fileSize) | (packet[3+i]<<(i*8));
  }
  if(packet[3+*fileSizeBytes] != T2){
    printf("Invalid Packet: File without name\n");
  }
  *fileNameSize = (int)packet[4+*fileSizeBytes];
  fileName = (char *)realloc(fileName,*fileNameSize);
  for(int i = 0; i < *fileNameSize;i++){
    fileName[i] = packet[5+(*fileSizeBytes)+i];
  }
}
