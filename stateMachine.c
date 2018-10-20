
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


int stateMachine(unsigned char c, int curr_state, unsigned char arg[]){
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
        //CTOP = TRUE;
        return curr_state=5;
      }
      else{
        curr_state = 0;
      }
      break;
  }
  return curr_state;
}


unsigned char readControlMessage(unsigned char c, int curr_state){
  unsigned char result=0;
  printf("State: %d\n", curr_state);
  switch(curr_state){
    case 0:
      if(c == FLAG){
        curr_state = 1;
      }
      break;
    case 1:
      if(c == FLAG){
        curr_state = 1;
      }
      else if(c == Arec){
        curr_state = 2;
      }
      else{
        curr_state = 0;
      }
      break;
    case 2:
      if(c == FLAG){
        curr_state = 1;
      }
      else if(c ==RR0 || c==RR1 || c==REJ0 || c==REJ1  || c==DISC){
        curr_state = 3;
        result=c;
      }
      else{
        curr_state = 0;
      }
      break;
    case 3:
      if(c == FLAG){
        curr_state = 1;
      }
      else if(c == (Arec ^ result)){
        curr_state = 4;
      }
      else{
        curr_state = 0;
      }
      break;
    case 4:
      if(c == FLAG){
        //CTOP = TRUE;
        curr_state=5;
      }
      else{
        curr_state = 0;
      }
      break;
    case 5:
      return result;
    default:
      break;
  }
  return result;

}