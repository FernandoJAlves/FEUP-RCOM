#include "reader.h"
#include "data_link.h"
#include "stateMachine.h"

void data_reader(int argc, char * argv[]){

    llopenR(1,2);

}

int checkBCC2(unsigned char *packet, int size)
{
  unsigned char byte = packet[0];
  for (int i = 0; i < size - 1; i++)
  {
    byte = byte^packet[i];
  }
  if (byte == packet[size - 1])
  {
    return 1;
  }
  else
    return 0;
}

int llread(unsigned char c, int curr_state, unsigned char arg[]){
  printf("State: %d\n", curr_state);
  switch(curr_state){
    case 0:
      if(c == arg[0]){
        curr_state = 1;
      }
      break;
    case 1:
      if(c == arg[0]){
        curr_state = 1;
      }
      else if(c == arg[1]){
        curr_state = 2;
      }
      else{
        curr_state = 0;
      }
      break;
    case 2:
      if(c == arg[0]){
        curr_state = 1;
      }
      else if(c == arg[2]){
        curr_state = 3;
      }
      else{
        curr_state = 0;
      }
      break;
    case 3:
      if(c == arg[0]){
        curr_state = 1;
      }
      else if(c == arg[3]){
        curr_state = 4;
      }
      else{
        curr_state = 0;
      }
      break;
    case 4:
      if(c == arg[0]){
        return curr_state=5;
      }
      else{
        curr_state = 0;
      }
      break;
  }
  return curr_state;
}


int llopenR(int porta, int status){
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
    if(link_layer.status){//RECETOR
        //TODO UA protocol
        //le o set
        while (curr_level<5) {       /* loop for input */
          res = read(fd,buf,1);
          if(res>0){
            curr_level=stateMachine(buf[0],curr_level,SET);
          }
        }
        ///SEND UA
        printf("sent UA");
        res=write(fd,UA,5);
    }
    return fd;
  }

