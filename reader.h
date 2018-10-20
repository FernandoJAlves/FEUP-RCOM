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

void data_reader(int argc, char * argv[]);

//       Data Link Layer Functions

int llopenR(int porta, int status);

unsigned char * llread(int fd, unsigned int  * size);

unsigned char SET[5];
unsigned char UA[5];
int fd;


//      Application Layer Functions






#endif
