/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include "macros.h"

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1



volatile int STOP=FALSE;

int receive_data(char * arg)
{


    unsigned char SET[5];
    SET[0] = FLAG;
    SET[1] = A;
    SET[2] = setC;
    SET[3]= SET[1]^SET[2];
    SET[4] = FLAG;

    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255],guardar[255];

    if ( (strcmp("/dev/ttyS0", arg)!=0) && 
  	      (strcmp("/dev/ttyS1", arg)!=0)) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


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
    leitura do(s) próximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");
    strcpy(guardar,"");

    while (STOP==FALSE) {       /* loop for input */

      res = read(fd,buf,1);   /* returns after 5 chars have been input */
      if(res > 0){
        buf[res]=0;               /* so we can printf... */
        printf(":%s:%d\n", buf, res);
      	strcat(guardar,buf);
        if (buf[res-1]=='\0') STOP=TRUE;
      }

    }
printf("%s\n",guardar);
    sleep(2);
	guardar[strlen(guardar)]='\0';
	res= write(fd,guardar,255);
	printf("enviado %d\n",res);
	


  /* 
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no guião 
  */

    sleep(2);

    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}



int SetstateMachine(unsigned char c, int curr_state, unsigned char SET[]){
  printf("State: %d\n", curr_state);
  switch(curr_state){
    case 0:
      if(c == SET[0]){
        curr_state = 1;
      }
      break;
    case 1:
      if(c == SET[0]){
        curr_state = 1;
      }
      else if(c == SET[1]){
        curr_state = 2;
      }
      else{
        curr_state = 0;
      }
      break;
    case 2:
      if(c == SET[0]){
        curr_state = 1;
      }
      else if(c == SET[2]){
        curr_state = 3;
      }
      else{
        curr_state = 0;
      }
      break;
    case 3:
      if(c == SET[0]){
        curr_state = 1;
      }
      else if(c == SET[3]){
        curr_state = 4;
      }
      else{
        curr_state = 0;
      }
      break;
    case 4:
      if(c == SET[0]){
        STOP = TRUE;
      }
      else{
        curr_state = 0;
      }
      break;
  }
  return curr_state;
}
