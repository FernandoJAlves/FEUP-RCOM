#include "writer.h"
#include "data_link.h"
#include "stateMachine.h"

int numAttempts = 0;
int isConnected = 0;
int openOrWrite = 0;
//handles the alarm signal
int flag = 0;

int tramaInfo = 0;

//main function called after choosing sender/receiver

int llwriteW(int fd, unsigned char *packetsFromCtrl, int sizeOfTrama)
{
  numAttempts = 0;
  isConnected = 0;
  flag = 1;
  //unsigned char * BCC2s = (unsigned char *)malloc(sizeof(unsigned char));
  finalMessage = (unsigned char *)malloc((sizeOfTrama + 6) * sizeof(unsigned char));
  finalSize = sizeOfTrama + 6;
  finalMessage[0] = FLAG;
  finalMessage[1] = Aemiss;

  if (!tramaInfo)
  {
    finalMessage[2] = nsC;
  }
  else
    finalMessage[2] = nsI;
  finalMessage[3] = finalMessage[1] ^ finalMessage[2];

  int numOfTramas;
  int j = 4;
  for (numOfTramas = 0; numOfTramas < sizeOfTrama; numOfTramas++)
  {
    //FLAG ou octedo 0x
    unsigned char readByte = packetsFromCtrl[numOfTramas];
    if (readByte == FLAG)
    { //stuffing
      finalMessage = (unsigned char *)realloc(finalMessage, ++finalSize);
      finalMessage[j] = ESCAPEBYTE;
      finalMessage[j + 1] = ESCAPE_FLAG1;
      j += 2;
    }
    else
    {
      if (readByte == ESCAPEBYTE)
      { //stuffing for escape
        finalMessage = (unsigned char *)realloc(finalMessage, ++finalSize);
        finalMessage[j] = ESCAPEBYTE;
        finalMessage[j + 1] = ESCAPE_FLAG2;
        j += 2;
      }
      else
      { // dados
        finalMessage[j] = packetsFromCtrl[numOfTramas];
        j++;
      }
    }
  }

  int sizeBCC2 = 1;
  unsigned char *BCC2Stuffed /*= (unsigned char *)malloc(sizeof(unsigned char))*/;
  unsigned char BCC2 = getBCC2(packetsFromCtrl, sizeOfTrama);
  BCC2Stuffed = stuffing(BCC2, &sizeBCC2);
  //if()
  if (sizeBCC2 == 1){
    finalMessage[j] = BCC2; //bcc ok
    printf("BCC2 normal\n");
  }

  else
  {
    finalMessage = (unsigned char *)realloc(finalMessage, ++finalSize);
    finalMessage[j] = BCC2Stuffed[0];
    finalMessage[j + 1] = BCC2Stuffed[1]; //
    j++;
    printf("BCC2 stuffed\n");
  }
  finalMessage[j + 1] = FLAG;
  int rej = 0;
  do
  {
    write(fd, finalMessage, finalSize);
    
    alarm(3);
    RRv[0]=FLAG;
    RRv[1]=Aemiss;
    RRv[2]=RR0; //não é usado
    RRv[3]=RRv[1]^RRv[2];
    RRv[4]=FLAG;

    unsigned char C = readControlMessageW(fd,RRv);
    
    if ((C == RR0 && tramaInfo == 1) || (C == RR1 && tramaInfo == 0))
    {
      rej = 0;
      numAttempts = 0;
      tramaInfo = (tramaInfo+1) % 2;
      if(C == RR0){
        printf("RR0 received\n");
      }
      else{
        printf("RR1 received\n");
      }
      alarm(0); //ALARM DEVE SALTAR FORA DO LOOP
      //isConnected = 1;
      if(packetsFromCtrl[0] == CTRL_C_END){
        break;
      }
      //break;

    }
    else if ((C == REJ0) || (C == REJ1))
    {
      rej = 1;
      if(C == REJ0){
        printf("REJ0 received\n");
      }
      else{
        printf("REJ1 received\n");
      }
      alarm(0);
      //break;
    }
    else
      return -1;
  }while (((!isConnected) && (numAttempts < 4)) || rej);

  return 1;
}

unsigned char getBCC2(unsigned char *mensagem, int size)
{
  unsigned char BCC2 = mensagem[0];
  int i;
  for (i = 1; i < size; i++)
  {
    BCC2 ^= mensagem[i];
  }
  return BCC2;
}

unsigned char *stuffing(unsigned char buff, int *size)
{
  unsigned char *returnValue;
  returnValue = (unsigned char *)malloc(2 * sizeof(unsigned char *));

  if (buff == FLAG){
    returnValue[0] = ESCAPEBYTE;
    returnValue[1] = ESCAPE_FLAG1;
    (*size)++;
  }
  else if (buff == ESCAPEBYTE){
    returnValue[0] = ESCAPEBYTE;
    returnValue[1] = ESCAPE_FLAG2;
    (*size)++;
  }

  return returnValue;
}

int llopenW(int porta, int status)
{
  (void)signal(SIGALRM, timeout);
  int res;
  SET[0] = FLAG;
  SET[1] = Aemiss;
  SET[2] = setC;
  SET[3] = SET[1] ^ SET[2];
  SET[4] = FLAG;

  UA[0] = FLAG;
  UA[1] = Aemiss;
  UA[2] = uaC;
  UA[3] = UA[1] ^ UA[2];
  UA[4] = FLAG;
  char buf[255];
  int curr_level = 0;

  /*
      Open serial port device for reading and writing and not as controlling tty
      because we don't want to get killed if linenoise sends CTRL-C.
    */
  fd = open(link_layer.port, O_RDWR | O_NOCTTY);
  if (fd < 0)
  {
    perror(link_layer.port);
    exit(-1);
  }
  if (setTermios(fd) < 0)
  {
    perror("Setting termios settings");
    return -1;
  }
  if (!link_layer.status)
  { //EMISSOR
    if ((!isConnected) && (numAttempts < 4))
    {
      res = write(fd, SET, 5);
      alarm(3);
    }
    printf("Sent SET,waiting for receiver\n");
    //RECEIVE UA
    while (curr_level < 5)
    { /* loop for input */
      res = read(fd, buf, 1);
      if (res > 0)
      {
        curr_level = stateMachine(buf[0], curr_level, UA);
      }
    }
    printf("Received UA\n");
  }

  isConnected = 1;

  return fd;
}

void callAlarm()
{
  if (!flag)
    write(fd, SET, 5);
  else
    write(fd, finalMessage, finalSize); // declared global variables malloc ca<refull
  alarm(3);
}
void timeout()
{
  numAttempts++;
  
  if(numAttempts > 4){
    exit(0);
  }
  printf("Attempt number=%d\n", numAttempts);
  callAlarm();
}





void llcloseW(int fd){
  sendControlField(fd, DISC);
  printf("Sent DISC\n");
  DISCw[0]=FLAG;
  DISCw[1]=Aemiss;
  DISCw[2]=DISC; //não é usado
  DISCw[3]=DISCw[1]^DISCw[2];
  DISCw[4]=FLAG;
  unsigned char returnValue = readControlMessageW(fd,DISCw);

  while(returnValue != DISC){
    returnValue = readControlMessageW(fd,DISCw);
  }

  printf("Received DISC\n");

  sendControlField(fd, uaC);
  printf("Last UA sent\n");

  if(tcsetattr(fd, TCSANOW, &link_layer.oldPortSettings) == -1){
    perror("tcsetattr");
    exit(-1);
  }


}



