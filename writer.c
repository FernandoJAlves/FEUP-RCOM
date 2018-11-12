#include "writer.h"
#include "data_link.h"
#include "stateMachine.h"
#include "utilities.h"

int numAttempts = 0;
int isConnected = 0;
int openOrWrite = 0;

//handles the alarm signal
int flag = 0;

int tramaInfo = 0;

int llwriteW(int fd, unsigned char *packetsFromCtrl, int sizeOfTrama)
{
  startCounter(); //Counter for elapsed time
  numAttempts = 0;
  isConnected = 0;
  flag = 1;
  finalMessage = (unsigned char *)malloc((sizeOfTrama + 6) * sizeof(unsigned char));
  finalSize = sizeOfTrama + 6;
  finalMessage[0] = FLAG;
  finalMessage[1] = Aemiss;

  if (tramaInfo == 0)
  {
    finalMessage[2] = C0;
  }
  else
    finalMessage[2] = C1;

  finalMessage[3] = finalMessage[1] ^ finalMessage[2]; //BCC1

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
      { // data that does not need stuffing
        finalMessage[j] = packetsFromCtrl[numOfTramas];
        j++;
      }
    }
  }

  int sizeBCC2 = 1;

  int rej = 0;

  do{
    
    unsigned char *BCC2Stuffed;
    unsigned char BCC2 = getBCC2(packetsFromCtrl, sizeOfTrama);
    BCC2Stuffed = stuffing(BCC2, &sizeBCC2);
    if (sizeBCC2 == 1){
      finalMessage[j] = BCC2; //bcc ok
      //printf("BCC2 normal\n");
    }

    else
    {
      finalMessage = (unsigned char *)realloc(finalMessage, ++finalSize);
      finalMessage[j] = BCC2Stuffed[0];
      finalMessage[j + 1] = BCC2Stuffed[1]; //
      j++;
      //printf("BCC2 stuffed\n");
    }
    finalMessage[j + 1] = FLAG;
    
    write(fd, finalMessage, finalSize);    
    
    RRv[0]=FLAG;
    RRv[1]=Aemiss;
     
    RRv[2]=RR0; //not used
    RRv[3]=RRv[1]^RRv[2]; //not used
    RRv[4]=FLAG; //not used

    alarm(3);

    unsigned char C = readControlMessageW(fd,RRv);
    
    if ((C == RR0 && tramaInfo == 1) || (C == RR1 && tramaInfo == 0)) //successful
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

      break;
    }
    else if ((C == REJ0) || (C == REJ1)) //rejected
    {
      rej = 1;
      if(C == REJ0){
        printf("REJ0 received\n");
      }
      else if (C == REJ1){
        printf("REJ1 received\n");
      }
    }
    else{ //unexpected return message
      printf("Invalid data received!\n");
    }
  }while (((!isConnected) && (numAttempts < MAXATTEMPTS)) || rej);
  printf("Transfer Rate: %.1f Kb/s\n",getTransferRate(sizeOfTrama));
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
  if(link_layer.n_wrong_packets > 0){
    link_layer.n_wrong_packets--;
    BCC2 = 0;
  }
  return BCC2;
}

unsigned char *stuffing(unsigned char buff, int *size)
{
  //stuffing of BCC2
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
  { //Emissor
    if ((!isConnected) && (numAttempts < 4))
    {
      res = write(fd, SET, 5);
      alarm(3);
    }
    printf("Sent SET,waiting for receiver\n");
    //Receive UA
    while (curr_level < 5)
    { 
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
  //alarm handler
  numAttempts++;
  
  if(numAttempts > MAXATTEMPTS){
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
  //sleep(1);
  sendControlField(fd, uaC);
  printf("Last UA sent\n");
  sleep(1);

  if(tcsetattr(fd, TCSANOW, &link_layer.oldPortSettings) == -1){
    perror("tcsetattr");
    exit(-1);
  }


}