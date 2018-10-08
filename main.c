#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "writenoncanonical.h"
#include "noncanonical.h"



int main(int argc, char** argv){
  char result;
   if ( (argc < 3) ||
        ((strcmp("/dev/ttyS0", argv[1])!=0) && (strcmp("/dev/ttyS0", argv[1])!=0)) ||
        ((strcmp("S", argv[2]) !=0) &&  (strcmp("R", argv[2]) !=0))) {

     printf("Usage:\tnserial SerialPort ComunicationMode\n\tex: nserial /dev/ttyS1 R\n");
     exit(1);
   }

  unsigned int state=0;
    if(strcmp(argv[2],"S")==0){
      printf("MODE: SENDER\n");
      result=send_data(argv[1]);
    }
    else{
          printf("MODE: RECEIVER\n");
      result=receive_data(argv[1]);
    }
   return state;
}
