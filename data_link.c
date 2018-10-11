#include "data_link.h"

int llopen(int porta, int status){
    int fd,res;
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
    if(link_layer.status){
     //send SET to statemachine
     unsigned char SET[5];
     SET[0] = FLAG;
     SET[1] = A;
     SET[2] = setC;
     SET[3]= SET[1]^SET[2];
     SET[4] = FLAG;
     printf("chegou");
     res=write(fd,SET,5);

     //waitforUA;
    }
    else{
        //TODO UA protocol
        //le o set
        unsigned char SET[5];
        SET[0] = FLAG;
        SET[1] = A;
        SET[2] = setC;
        SET[3]= SET[1]^SET[2];
        SET[4] = FLAG;
        char buf[255];
        int curr_level=0;
        while (curr_level<5) {       /* loop for input */
          res = read(fd,buf,1);
          if(res>0){
            curr_level=stateMachine(buf[0],curr_level,SET);
            printf("\nnivel %d",curr_level);
          }
        }
        // res=write(fd,UA,5);
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
