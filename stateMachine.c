
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

int stateMachine(unsigned char c, int curr_state, unsigned char arg[])
{
  //state machine for SET/UA 
  switch (curr_state)
  {
  case 0:
    if (c == arg[0])
    {
      curr_state = 1;
    }
    break;
  case 1:
    if (c == arg[0])
    {
      curr_state = 1;
    }
    else if (c == arg[1])
    {
      curr_state = 2;
    }
    else
    {
      curr_state = 0;
    }
    break;
  case 2:
    if (c == arg[0])
    {
      curr_state = 1;
    }
    else if (c == arg[2])
    {
      curr_state = 3;
    }
    else
    {
      curr_state = 0;
    }
    break;
  case 3:
    if (c == arg[0])
    {
      curr_state = 1;
    }
    else if (c == arg[3])
    {
      curr_state = 4;
    }
    else
    {
      curr_state = 0;
    }
    break;
  case 4:
    if (c == arg[0])
    {
      return curr_state = 5;
    }
    else
    {
      curr_state = 0;
    }
    break;
  }
  return curr_state;
}



unsigned char readControlMessageW(int fd,unsigned char * ctrl)
{
  int curr_state = 0;
  unsigned char c, returnValue;
  while (curr_state != 5)
  {
    read(fd, &c, 1);
    
    switch (curr_state)
    {
    case 0:
      if (c == ctrl[0])
      {
        curr_state = 1;
      }
      break;
    case 1:
      if (c == ctrl[0])
      {
        curr_state = 1;
      }
      else if (c == ctrl[1])
      {
        curr_state = 2;
      }
      else
      {
        curr_state = 0;
      }
      break;
    case 2:
      if (c == ctrl[0])
      {
        curr_state = 1;
      }
      else if (c == RR0 || c == RR1 || c == REJ0 || c == REJ1 || c == DISC)
      {
        curr_state = 3;
        returnValue = c;
      }
      else
      {
        curr_state = 0;
      }
      break;
    case 3:
      if (c == ctrl[0])
      {
        curr_state = 1;
      }
      else if (c == (ctrl[1] ^ returnValue))
      {
        curr_state = 4;
      }
      else
      {
        curr_state = 0;
      }
      break;
    case 4:
    
      if (c == ctrl[0])
      {
        
        curr_state = 5;
        return returnValue;
      }
      else
      {
        curr_state = 0;
      }
      break;
    default:
      break;
    }
  }
  return returnValue;
}

unsigned char readControlMessageR(int fd,unsigned char * ctrl)
{
  //reads DISC/UA in reader
  int curr_state = 0;
  unsigned char c;
  while (curr_state != 5)
  {
    
    read(fd, &c, 1);
    
    switch (curr_state)
    {
    case 0:
      if (c == ctrl[0])
      {
        curr_state = 1;
      }
      break;
    case 1:
      if (c == ctrl[0])
      {
        curr_state = 1;
      }
      else if (c == ctrl[1])
      {
        curr_state = 2;
      }
      else
      {
        curr_state = 0;
      }
      break;
    case 2:
      if (c == ctrl[0])
      {
        curr_state = 1;
      }
      else if (c == ctrl[2])
      {
        curr_state = 3;
      }
      else
      {
        curr_state = 0;
      }
      break;
    case 3:
      if (c == ctrl[0])
      {
        curr_state = 1;
      }
      else if (c == (ctrl[1] ^ ctrl[2]))
      {
        curr_state = 4;
      }
      else
      {
        curr_state = 0;
      }
      break;
    case 4:
      if (c == ctrl[0])
      {
        curr_state = 5;
        return 0;
      }
      else
      {
        curr_state = 0;
      }
      break;
    default:
      break;
    }
  }
  return 0;
}

