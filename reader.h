#ifndef READER_H_
#define READER_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <signal.h>



//       Data Link Layer Functions

int llopenR(int porta, int status);

unsigned char * llread(int fd, unsigned long  * size);

void llcloseR(int fd);

int checkBCC2(unsigned char *packet, int size);

// Global variables

int fd;
int expectedBCC;







#endif
