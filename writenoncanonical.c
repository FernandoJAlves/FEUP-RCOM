/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

unsigned char SET[5];
SET[0] = FLAG;
SET[1] = A;
SET[2] = setC;
SET[3]= SET[1]^SET[2];
SET[4] = FLAG;

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];
    char msg[255];
    int i, sum = 0, speed = 0;
    
    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }
    printf("Write your message: ");
    if(gets(buf) == NULL){
      perror("Error reading message");
      exit(1);
    }
    printf("%d bytes written\n", strlen(buf));
  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */


    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

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


/*
    for (i = 0; i < 255; i++) {
      buf[i] = 'a';
    }
    buf[0] = 'z';
    buf[254] = '\0';
    
    buf[25] = '\n';
    
    res = write(fd,buf,255);   
    printf("%d bytes written\n", res);
 */

  /* 
    O ciclo FOR e as instru��es seguintes devem ser alterados de modo a respeitar 
    o indicado no gui�o 
  */

  write(fd,SET,5);
  alarm(3);

  responseRequestConnection(fd);  

  printf("%d bytes written\n", res);




  sleep(2);
  strcpy(msg,"");

  while (STOP==FALSE) {       /* loop for input */
      res = read(fd,buf,1);   /* returns after 5 chars have been input */
      if(res > 0){
         buf[res]=0;               /* so we can printf... */
        printf(":%s:%d\n", buf, res);
        strcat(msg,buf);
        if (buf[res]=='\0') STOP=TRUE;
      }
    }

    printf("%s",msg);


      

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }


    close(fd);
    return 0;
}

unsigned char UA[5];
UA[0]=FLAG;
UA[1]=A;
UA[2]=uaC;
UA[3]=A^uaC;
UA[4]=FLAG;



void responseRequestConnection(int fd){
  unsigned char c;
  int state=0;
  while(!connectionEstabilished){
    read(fd,&c,1);
    switch(state){
      //recebe flag
      case 0:
        if(c==UA[0])
          state=1;
          break;
      //recebe A
      case 1:
        if(c==UA[1])
          state=2;
        else
          {
            if(c==UA[0])
              state=1;
            else
              state = 0;
          }
      break;
      //recebe C
      case 2:
        if(c==UA[2])
          state=3;
        else{
          if(c==UA[0])
            state=1;
          else
            state = 0;
        }
      break;
      //recebe BCC
      case 3:
        if(c==UA[3])
          state = 4;
        else
          state=0;
      break;
      //recebe FLAG final
      case 4:
        if(c==UA[4])
          connectionEstabilished=1;
        else
          state = 0;
      break;
    }
  }
}
