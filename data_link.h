#ifndef DATA_LINK_H_
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>
#include "macros.h"
#define BAUDRATE B38400
#define TRANSMITTER 0
#define RECEIVER 1
#define COM_1 "/dev/ttyS0"
#define COM_2 "/dev/ttyS1"
typedef struct{
    char * port; /*Dispositivo /dev/ttySx, x = 0, 1*/
    int baudRate; /*Velocidade de transmissão*/
    unsigned int sequenceNumber;   /*Número de sequência da trama: 0, 1*/
    unsigned int timeout; /*Valor do temporizador: 1 s*/
    unsigned int numTransmissions; /*Número de tentativas em caso de falha*/
    int status;
    struct termios oldPortSettings;
} LinkLayer;
LinkLayer link_layer;



int setTermios(int fd);
int llopen(int porta, int status);
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


int llread(int fd, char * buffer);
// argumentos
// – fd: identificador da ligação de dados
// – buffer: array de caracteres recebidos
// retorno
// – comprimento do array
// (número de caracteres lidos)
// – valor negativo em caso de erro


#endif
