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
  printf("Trama info: %x\n", tramaInfo);
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
  unsigned char *BCC2Stuffed = (unsigned char *)malloc(sizeof(unsigned char));
  unsigned char BCC2 = getBCC2(packetsFromCtrl, sizeOfTrama);
  BCC2Stuffed = stuffing_BCC2(BCC2, &sizeBCC2);
  //if()
  if (sizeBCC2 == 1)
    finalMessage[j] = BCC2; //bcc ok
  else
  {
    finalMessage = (unsigned char *)realloc(finalMessage, ++finalSize);
    finalMessage[j] = BCC2Stuffed[0];
    finalMessage[j + 1] = BCC2Stuffed[1]; //
    j++;
  }
  finalMessage[j + 1] = FLAG;

  while ((!isConnected) && (numAttempts < 4))
  {
    write(fd, finalMessage, finalSize);
    alarm(3);
    unsigned char C = readControlMessage(fd);
    printf("C=%x\n", C);
    if ((C == RR0 && tramaInfo == 1) || (C == RR1 && tramaInfo == 0))
    {
      numAttempts = 0;
      tramaInfo ^= 1;
      printf("trama valida %x", C);
      alarm(0);
    }
    else if ((C == REJ0) || (C == REJ1))
    {
      printf("trama invalida %x", C);
      alarm(0);
    }
    else
      return -1;
  }
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

unsigned char *stuffing_BCC2(unsigned char BCC2, int *sizeOfBCC2)
{
  unsigned char *returnValue;
  returnValue = (unsigned char *)malloc(2 * sizeof(unsigned char *));

  if (BCC2 == FLAG){
    returnValue[0] = ESCAPEBYTE;
    returnValue[1] = ESCAPE_FLAG1;
    (*sizeOfBCC2)++;
  }
  else if (BCC2 == ESCAPEBYTE){
    returnValue[0] = ESCAPEBYTE;
    returnValue[1] = ESCAPE_FLAG2;
    (*sizeOfBCC2)++;
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
  UA[1] = Arec;
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
  printf("Attempt number=%d\n", numAttempts);
  callAlarm();
}

unsigned char readControlMessage(int fd)
{
  //unsigned char result = 0;
  int curr_state = 0;
  unsigned char c, returnValue;
  while (curr_state != 5)
  {
    read(fd, &c, 1);
    switch (curr_state)
    {
    case 0:
      if (c == FLAG)
      {
        curr_state = 1;
      }
      break;
    case 1:
      if (c == FLAG)
      {
        curr_state = 1;
      }
      else if (c == Aemiss)
      {
        curr_state = 2;
      }
      else
      {
        curr_state = 0;
      }
      break;
    case 2:
      if (c == FLAG)
      {
        curr_state = 1;
      }
      else if (c == RR0 || c == RR1 || c == REJ0 || c == REJ1 || c == DISC)
      {
        curr_state = 3;
        returnValue = c;
      }
      else
      {
        curr_state = 0;
      }
      break;
    case 3:
      if (c == FLAG)
      {
        curr_state = 1;
      }
      else if (c == (Arec ^ returnValue))
      {
        curr_state = 4;
      }
      else
      {
        curr_state = 0;
      }
      break;
    case 4:
      if (c == FLAG)
      {
        //TODO
        curr_state = 5;
      }
      else
      {
        curr_state = 0;
      }
      break;
    default:
      break;
    }
  }
  return returnValue;
}
