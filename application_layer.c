#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "macros.h"
#include "stateMachine.h"
#include "data_link.h"
#include "application_layer.h"



int main(int argc, char** argv){
    if ( (argc < 3) ||
        ((strcmp("1", argv[1])!=0) && (strcmp("2", argv[1])!=0)) ||
        ((strcmp("S", argv[2]) !=0) &&  (strcmp("R", argv[2]) !=0))) {

     printf("Usage:\tnserial SerialPort ComunicationMode\n\tex: nserial [1|2] [S|R]\n");
     exit(1);
    }

  
    if(!strcmp(argv[1],"1")){
      link_layer.port=COM_1;
    }
    else if(!strcmp(argv[1],"2")){
      link_layer.port=COM_2;
    }
    else{
      perror("porta 1 ou 2 \n");
    }
    if(!strcmp(argv[2],"S")){
      link_layer.status=TRANSMITTER;
    
    }
    else if(!strcmp(argv[2],"R")){
      link_layer.status=RECEIVER;
    }
    else{
      perror("S or R\n");
      return -1;
    }

    return 0;
}
