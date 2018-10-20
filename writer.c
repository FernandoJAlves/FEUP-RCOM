#include "writer.h"
#include "data_link.h"
#include "stateMachine.h"

int numAttempts=0;
int isConnected=0;

//handles the alarm signal
void timeout(){

  if((!isConnected) && (numAttempts < 4))
  {
    numAttempts++;
    printf("Attempt number=%d\n", numAttempts);
    connect();
  }
  else
  {
    printf("Operation failed!\n");
    exit(-1);
  }

}

//main function called after choosing sender/receiver

int llwriteW(int fd, unsigned char * packetsFromCtrl,int sizeOfTrama){
  //unsigned char * BCC2s = (unsigned char *)malloc(sizeof(unsigned char));
  unsigned char * finalMessage= (unsigned char *)malloc((sizeOfTrama+6) * sizeof(unsigned char));
  int finalSize=sizeOfTrama+6;
  finalMessage[0]=FLAG;
  finalMessage[1]=Aemiss;
  if(!packetsFromCtrl[0]){
    finalMessage[2]=nsC;
  }
  else  finalMessage[2]=nsI;
  finalMessage[3]=finalMessage[1]^finalMessage[2];

  int numOfTramas;
  int j=4;
  for(numOfTramas=0;numOfTramas<sizeOfTrama;numOfTramas++){
    //FLAG ou octedo 0x
    unsigned char readByte=packetsFromCtrl[numOfTramas];
    if(readByte==FLAG){ //stuffing
      finalMessage = (unsigned char *)realloc(finalMessage, ++finalSize);
      finalMessage[j]=ESCAPEBYTE;
      finalMessage[j+1]=ESCAPE_FLAG1;
      j+=2;
    }
    else{
      if(readByte==ESCAPEBYTE){ //stuffing for escape
        finalMessage = (unsigned char *)realloc(finalMessage, ++finalSize);
        finalMessage[j]=ESCAPEBYTE;
        finalMessage[j+1]=ESCAPE_FLAG2;
        j+=2;
      }
      else{         // dados
        finalMessage[j]=packetsFromCtrl[numOfTramas];
        j++;
      }
    }
  }
  int sizeBCC2=1;
  unsigned char *BCC2Stuffed=malloc(sizeof(unsigned char));
  unsigned char BCC2=getBCC2(packetsFromCtrl,sizeOfTrama);
  BCC2Stuffed=stuffingBCC2(BCC2, &sizeBCC2);
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


  return 0;
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

unsigned char *stuffingBCC2(unsigned char BCC2, int *sizeBCC2)
{
  unsigned char *BCC2Stuffed;
  if (BCC2 == FLAG)
  {
    BCC2Stuffed = (unsigned char *)malloc(2 * sizeof(unsigned char *));
    BCC2Stuffed[0] = ESCAPEBYTE;
    BCC2Stuffed[1] = ESCAPE_FLAG1;
    (*sizeBCC2)++;
  }
  else
  {
    if (BCC2 == ESCAPEBYTE)
    {
      BCC2Stuffed = (unsigned char *)malloc(2 * sizeof(unsigned char *));
      BCC2Stuffed[0] = ESCAPEBYTE;
      BCC2Stuffed[1] = ESCAPE_FLAG2;
      (*sizeBCC2)++;
    }
  }

  return BCC2Stuffed;
}


int llopenW(int porta, int status){
    int res;
    SET[0] = FLAG;
    SET[1] = Aemiss;
    SET[2] = setC;
    SET[3]= SET[1]^SET[2];
    SET[4] = FLAG;

    UA[0] = FLAG;
    UA[1] = Arec;
    UA[2] = uaC;
    UA[3]= UA[1]^UA[2];
    UA[4] = FLAG;
    char buf[255];
    int curr_level=0;

    /*
      Open serial port device for reading and writing and not as controlling tty
      because we don't want to get killed if linenoise sends CTRL-C.
    */
    fd = open(link_layer.port, O_RDWR | O_NOCTTY );
    if (fd <0) {perror(link_layer.port); exit(-1); }
    if(setTermios(fd)<0){
      perror("Setting termios settings");
      return -1;
    }
    if(!link_layer.status){//EMISSOR
     (void) signal(SIGALRM,timeout);
     res=write(fd,SET,5);
     alarm(3);
     printf("Sent SET,waiting for receiver\n");
    //RECEIVE UA
     while (curr_level<5) {       /* loop for input */
        res = read(fd,buf,1);
        if(res>0){
          curr_level=stateMachine(buf[0],curr_level,UA);
        }
      }
          printf("Received UA\n");
    }

    isConnected = 1;

    return fd;
  }

void connect(){
    write(fd,SET,5);
    alarm(3);
}







