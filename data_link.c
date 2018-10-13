#include "data_link.h"
#include "stateMachine.h"
#include <signal.h>


int numAlarms=0;
int isConnected=0;

void timeout(){

  printf("Alarm=%d", numAlarms);

  //se ainda nao tivermos recebido UA e a soma dos alarmes for inferior a 4
  if((!isConnected) && (numAlarms < 4))
  {
    numAlarms++;
    connect();
  }
  else
  {
    printf("Operation failed!\n");
    exit(-1);
  }

}

 void connect(){
    write(fd,SET,5);
    alarm(3);
 }

int llopen(int porta, int status){
    int res;
    SET[0] = FLAG;
    SET[1] = A;
    SET[2] = setC;
    SET[3]= SET[1]^SET[2];
    SET[4] = FLAG;

    UA[0] = FLAG;
    UA[1] = A;
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
      perror("setting termios settings");
      return -1;
    }
    if(!link_layer.status){//EMISSOR
     (void) signal(SIGALRM,timeout);
     res=write(fd,SET,5);
     alarm(3);
     printf("Sent SET,waiting for receiver\n");
    //RECIEVE UA
     while (curr_level<5) {       /* loop for input */
        res = read(fd,buf,1);
        if(res>0){
          curr_level=stateMachine(buf[0],curr_level,UA);
        }
      }
          printf("received UA");
    }
    else{//RECETOR
        //TODO UA protocol
        //le o set
        while (curr_level<5) {       /* loop for input */
          res = read(fd,buf,1);
          if(res>0){
            curr_level=stateMachine(buf[0],curr_level,SET);
          }
        }
        isConnected = 1;
        ///SEND UA
        printf("sent UA");
        res=write(fd,UA,5);
    }
    return fd;
  }


int setTermios(int fd){
  struct termios oldtio,newtio;
  if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
  }
  link_layer.oldPortSettings=oldtio;
  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
  newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */
/*
  VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
  leitura do(s) pr�ximo(s) caracter(es)
*/
  tcflush(fd, TCIOFLUSH);

  if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");
  return 0;
}
