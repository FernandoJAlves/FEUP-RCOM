#include "data_link.h"

int llopen(int porta, int status){
    int fd;
    struct termios oldtio,newtio;
    if(TRANSMITTER){
    /*
      Open serial port device for reading and writing and not as controlling tty
      because we don't want to get killed if linenoise sends CTRL-C.
    */
      fd = open(link_layer.port, O_RDWR | O_NOCTTY );
      if (fd <0) {perror(link_layer.port); exit(-1); }

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
    }
    else{
      fd = open(link_layer.port, O_RDWR | O_NOCTTY );
      if (fd <0) {perror(link_layer.port); exit(-1); }

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
    }
    return 0;
  }