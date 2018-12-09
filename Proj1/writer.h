#ifndef WRITER_H_
#define WRITER_H_

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

#define CTRL_C_START 0x02
#define CTRL_C_END   0x03


void callAlarm();
void timeout();
//       Data Link Layer Functions

int llopenW(int porta, int status); //TODO Tirar argumentos
// argumentos
// –porta: COM1, COM2, ...
// –flag: TRANSMITTER / RECEIVER
// retorno
// –identificador da ligação de dados
// –valor negativo em caso de erro


int llwrite(int fd, char * buffer, int length);
// argumentos
// – fd: identificador da ligação de dados
// – buffer: array de caracteres a transmitir
// – length: comprimento do array de caracteres
// retorno
// – número de caracteres escritos
// – valor negativo em caso de erro


//Global variables
int finalSize;
unsigned char * finalMessage;
int fd;
int llwriteW(int fd, unsigned char * startOfFile,int finalSize);

//      Application Layer Functions


unsigned char getBCC2(unsigned char *mensagem, int size);
unsigned char *stuffing(unsigned char BCC2, int *sizeOfBCC2);

void llcloseW(int fd);

#endif
