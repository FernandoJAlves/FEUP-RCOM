#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "macros.h"
#include "stateMachine.h"
#include "data_link.h"
#include "application_layer.h"
#include "writer.h"
#include "reader.h"




int main(int argc, char** argv){
    if ( (argc < 3) ||
        ((strcmp("1", argv[1])!=0) && (strcmp("2", argv[1])!=0)) ||
        ((strcmp("S", argv[2]) !=0) &&  (strcmp("R", argv[2]) !=0))) {

     printf("Usage:\tnserial SerialPort ComunicationMode\n\tex: nserial [1|2] [S|R]\n");
     exit(1);
    }


    if(!strcmp(argv[1],"1")){
      link_layer.port=COM_1;
    }
    else if(!strcmp(argv[1],"2")){
      link_layer.port=COM_2;
    }
    else{
      perror("porta 1 ou 2 \n");
      return -1;
    }
    if(!strcmp(argv[2],"S")){
      link_layer.status=TRANSMITTER;
      data_writer(argc, argv);

    }
    else if(!strcmp(argv[2],"R")){
      link_layer.status=RECEIVER;
      data_reader(argc, argv);
    }
    else{
      perror("S or R\n");
      return -1;
    }

    return 0;
}

void data_writer(int argc, char * argv[]){

    int fd;

    fd=llopenW(1,2);


    int file_name_size = strlen(pinguim);
    unsigned char * file_name = (unsigned char *)malloc(file_name_size);
    file_name = (unsigned char *)pinguim;
    off_t final_size;
    unsigned char * read_file=readFile(file_name,&final_size);
    unsigned char * pointerToCtrlPacket=makeControlPackage_I(final_size,file_name, file_name_size,(int *) &final_size,CTRL_C_START); 
    printf("size of File %ld  controlPackageadress %x \n",sizeof(final_size),pointerToCtrlPacket);
    llwriteW(fd,read_file,final_size);
}

void data_reader(int argc, char * argv[]){

    int fd=llopenR(1,2);
    unsigned int size;
    unsigned char * finalPacket=llread(fd,&size);
    printf("\nsize of file received in bytes: %lu\n",sizeof(finalPacket)/sizeof(finalPacket[0]));

}


unsigned char * makeControlPackage_I(off_t fileSize, unsigned char * fileName, int fileName_size, int * finalSize, unsigned char start_or_end){
  /*

TLV (Type, Length, Value)
– T (um octeto) – indica qual o parâmetro (0 – tamanho do ficheiro, 1 – nome do
ficheiro, outros valores – a definir, se necessário)
– L (um octeto) – indica o tamanho em octetos do campo V (valor do parâmetro)
– V (número de octetos indicado em L) – valor do parâmetro
  */
  
  
  	int start_packet_len = 5 + sizeof(fileSize) + fileName_size;
  	char *finalPackage = ( char *)malloc(sizeof(char) * start_packet_len);

    if(start_or_end == CTRL_C_START){
      finalPackage[0]=start_or_end;
    }
    else if(start_or_end == CTRL_C_END){
      finalPackage[0] = start_or_end;
    }
    else {printf("Invalid value in start_or_end!");return NULL;}
    finalPackage[1]=T1; //Tamanho do ficheiro
    finalPackage[2]=sizeof(fileSize);  //8
	*((off_t *)(finalPackage + 3)) = fileSize;
  	finalPackage[3 + sizeof(fileSize)] = T2;  //Nome do ficheiro
  	finalPackage[4 + sizeof(fileSize)] = fileName_size;
	strcat(finalPackage + 5 + sizeof(fileSize),(char *)fileName);
	printf("size of fileSize:%ld    size of finalPackage:%ld",fileSize,sizeof(finalPackage[4]));

   // finalPackage[3]=
    return NULL;

}



unsigned char * readFile(unsigned char * fileName, off_t * fileSize){

    FILE * fd;
    struct stat data;

    if((fd= fopen(((const char *)fileName), "rb")) == NULL){
        perror("Error while opening the file");
        return NULL;
    }
    stat((const char*)fileName, &data); //get the file metadata

    *fileSize = data.st_size; //gets file size in bytes


    unsigned char * fileData = (unsigned char*)malloc(*fileSize);

    fread(fileData,sizeof(unsigned char),*fileSize,fd);
    if(ferror(fd)){
          perror("error writting to file\n");
    }
    fclose(fd);
    return fileData;

}