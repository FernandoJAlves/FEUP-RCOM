#include "data_link.h"
#include "stateMachine.h"
#include <signal.h>

int setTermios(int fd) {
  struct termios oldtio,newtio;
  if (tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
    perror("tcgetattr");
    exit(-1);
  }
  link_layer.oldPortSettings=oldtio;
  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = link_layer.baudRate | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
  newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 char received */
  /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) pr�ximo(s) caracter(es)
  */
  tcflush(fd, TCIOFLUSH);

  if (tcsetattr(fd,TCSANOW,&newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");
  return 0;
}

int sendControlField(int fd, unsigned char controlField) {
  unsigned char controlPacket[5];
  controlPacket[0] = FLAG;
  controlPacket[1] = Aemiss;
  controlPacket[2] = controlField;
  controlPacket[3] = controlPacket[1] ^ controlPacket[2];
  controlPacket[4] = FLAG;
  int bytes = write(fd, controlPacket, 5);
  return bytes;
}

void set_n_wrong_packets(int n) {
  link_layer.n_wrong_packets = BER * n;
}
