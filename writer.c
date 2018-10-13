#include "writer.h"

int numAttempts=0;
int isConnected=0;

void timeout(){

  if((!isConnected) && (numAttempts < 4))
  {
    numAttempts++;
    printf("Attempt number=%d\n", numAttempts);
    connect();
  }
  else
  {
    printf("Operation failed!\n");
    exit(-1);
  }

}

void data_writer(int argc, char * argv[]){


}
