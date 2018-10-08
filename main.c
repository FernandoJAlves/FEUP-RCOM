#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "writenoncanonical.h"
#include "noncanonical.h"
#include "macros.h"
#include "stateMachine.h"



int main(int argc, char** argv){
  char result;
   if ( (argc < 3) ||
        ((strcmp("/dev/ttyS0", argv[1])!=0) && (strcmp("/dev/ttyS0", argv[1])!=0)) ||
        ((strcmp("S", argv[2]) !=0) &&  (strcmp("R", argv[2]) !=0))) {

     printf("Usage:\tnserial SerialPort ComunicationMode\n\tex: nserial /dev/ttyS1 R\n");
     exit(1);
   }

 unsigned char SET[5];
    SET[0] = FLAG;
    SET[1] = A;
    SET[2] = setC;
    SET[3]= SET[1]^SET[2];
    SET[4] = FLAG;
  unsigned int state=0;
  while(state!=5){
    if(strcmp(argv[2],"S")==0){
      printf("MODE: SENDER\n");
      result=send_data(argv[1]);
    }
    else{
          printf("MODE: RECEIVER\n");
      result=receive_data(argv[1]);
    }
  state=stateMachine(result, state, SET);
  }
   return state;
}
