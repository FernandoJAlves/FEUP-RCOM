/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>
#include "macros.h"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1


volatile int CTOP=FALSE;


int send_data(char * arg)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];
    char msg[255];
    int i, sum = 0, speed = 0;

    unsigned char UA[5];
    UA[0] = FLAG;
    UA[1] = A;
    UA[2] = uaC;
    UA[3]= UA[1]^UA[2];
    UA[4] = FLAG;
    
    if ( (strcmp("/dev/ttyS0", arg)!=0) && 
  	      (strcmp("/dev/ttyS1", arg)!=0)) {
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


    fd = open(arg, O_RDWR | O_NOCTTY );
    if (fd <0) {perror(arg); exit(-1); }

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

  //write(fd,UA,5);
  //alarm(3);

  printf("%d bytes written\n", res);




  sleep(2);
  strcpy(msg,"");

  while (CTOP==FALSE) {       /* loop for input */
      res = read(fd,buf,1);   /* returns after 5 chars have been input */
      if(res > 0){
         buf[res]=0;               /* so we can printf... */
        printf(":%s:%d\n", buf, res);
        strcat(msg,buf);
        if (buf[res]=='\0') CTOP=TRUE;
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

int UastateMachine(unsigned char c, int curr_state, unsigned char UA[]){
  printf("State: %d\n", curr_state);
  switch(curr_state){
    case 0:
      if(c == UA[0]){
        curr_state = 1;
      }
      break;
    case 1:
      if(c == UA[0]){
        curr_state = 1;
      }
      else if(c == UA[1]){
        curr_state = 2;
      }
      else{
        curr_state = 0;
      }
      break;
    case 2:
      if(c == UA[0]){
        curr_state = 1;
      }
      else if(c == UA[2]){
        curr_state = 3;
      }
      else{
        curr_state = 0;
      }
      break;
    case 3:
      if(c == UA[0]){
        curr_state = 1;
      }
      else if(c == UA[3]){
        curr_state = 4;
      }
      else{
        curr_state = 0;
      }
      break;
    case 4:
      if(c == UA[0]){
        CTOP = TRUE;
      }
      else{
        curr_state = 0;
      }
      break;
  }
  return curr_state;
}
