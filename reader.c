#include "reader.h"
#include "data_link.h"
#include "stateMachine.h"


/*
Tramas I recebidas sem erros detectados no cabeçalho e no campo de dados
são aceites para processamento
– Se se tratar duma nova trama, o campo de dados é aceite (e passado à Aplicação),
e a trama deve ser confirmada com RR
– Se se tratar dum duplicado, o campo de dados é descartado, mas deve fazer-se
confirmação da trama com RR
» Tramas I sem erro detectado no cabeçalho mas com erro detectado (pelo
respectivo BCC) no campo de dados – o campo de dados é descartado, mas o
campo de controlo pode ser usado para desencadear uma acção adequada
– Se se tratar duma nova trama, é conveniente fazer um pedido de retransmissão
com REJ, o que permite antecipar a ocorrência de time-out no em

e no interior da trama ocorrer o octeto 01111110 (0x7e), isto é, o padrão que
corresponde a uma flag, o octeto é substituído pela sequência 0x7d 0x5e (octeto
de escape seguido do resultado do ou exclusivo do octeto substituído com o octeto
0x20)
– Se no interior da trama ocorrer o octeto 01111101 (0x7d), isto é, o padrão que
corresponde ao octeto de escape, o octeto é substituído pela sequência 0x7d 0x5d
(octeto de escape seguido do resultado do ou exclusivo do octeto substituído com
o octeto 0x20)

*/
void data_reader(int argc, char * argv[]){

    int fd=llopenR(1,2);
    unsigned int size=1000;
    unsigned char * finalPacket=llread(fd,&size);
    printf("\nsize of file received in bytes: %lu\n",sizeof(finalPacket)/sizeof(finalPacket[0]));

}

int checkBCC2(unsigned char *packet, int size)
{
  unsigned char byte = packet[0];
  for (int i = 1; i < size - 1; i++){
      byte = byte^packet[i];
  }
  if (byte == packet[size - 1]){
    return 1;
  }
  else return 0;
}

int sendC(int fd, unsigned char controlField){
  unsigned char controlPacket[5];
  controlPacket[0] = FLAG;
  controlPacket[1] = Arec;
  controlPacket[2] = controlField;
  controlPacket[3] = controlPacket[1] ^ controlPacket[2];
  controlPacket[4] = FLAG;
  int bytes=write(fd, controlPacket, 5);
  return bytes;

}

unsigned char * llread(int fd, unsigned int  * size){
  int curr_state = 0;
  int sucess = 0;
  int tramaNum = 0;
  unsigned char controlField;
  unsigned char c;
  unsigned char *frame = (unsigned char *)malloc(0);
  int bccCheckedData;
  while(sucess){
	read(fd,&c,1);
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
		  else if(c == Aemiss){
		    curr_state = 2;
		  }
		  else{
		    curr_state = 0;
		  }
		break;
		case 2:
		  if(c == C0){
		    controlField = c;
		    tramaNum = 0;
		    curr_state = 3;
		  }
		  else if(c == C1){
		    controlField = c;
		    tramaNum = 1;
		    curr_state = 3;
		  }
		  else{
		    if (c == FLAG)
		      curr_state = 1;
		    else
		      curr_state = 0;
		  }
	    break;
		case 3:
		if (c == (Aemiss ^ controlField))
		  curr_state = 4;
		else
		  curr_state = 0;
		break;
		case 4:
		  if (c == FLAG){
				if (checkBCC2(frame, *size)){
				  if (tramaNum == 0)
				    sendC(fd, RR0);
				  else
				    sendC(fd, RR1);

				  curr_state = 6;
				  bccCheckedData = 1;
				  printf("Enviou RR, T: %d\n", tramaNum);
				}
				else{
				  if (tramaNum == 0)
				    sendC(fd, REJ0);
				  else
				    sendC(fd, REJ1);
				  curr_state = 6;
				  bccCheckedData = 0;
				  printf("Enviou REJ, T: %d\n", tramaNum);
				}
		  }
		  else if (c == ESCAPEBYTE){ //goes to state 5 for byte de-stuffing
		    curr_state = 5;
		  }
		  else{ //reads data
		    (*size)++;
		    frame = (unsigned char *)realloc(frame, *size);
		    frame[*size - 1] = c;
		  }
		break;
		case 5:
		  //BYTE DE-STUFFING
		  if (c == ESCAPE_FLAG1)
		  {
		    frame = (unsigned char *)realloc(frame, ++(*size));
		    frame[*size - 1] = FLAG;
		  }
		  else
		  {
		    if (c == ESCAPE_FLAG2)
		    {
		      frame = (unsigned char *)realloc(frame, ++(*size));
		      frame[*size - 1] = ESCAPEBYTE;
		    }
		    else
		    {
		      perror("Non valid character after escape character");
		      exit(-1);
		    }
		  }
		  curr_state = 4;
	  	break;
		case 6:
			sucess=1;
		break;
  	}
  }
  frame = (unsigned char *)realloc(frame, *size - 1);

  *size = *size - 1;
  if (bccCheckedData)
  {

  }

	return frame;
}



int llopenR(int porta, int status){
    int res;
    SET[0] = FLAG;
    SET[1] = Aemiss;
    SET[2] = setC;
    SET[3]= SET[1]^SET[2];
    SET[4] = FLAG;

    UA[0] = FLAG;
    UA[1] = Arec;
    UA[2] = uaC;
    UA[3]= UA[1]^UA[2];
    UA[4] = FLAG;
    char buf[255];
    int curr_level=0;
    /*
      Open serial port device for reading and writing and not as controlling tty
      because we don't want to get killed if linenoise sends CTRL-C.
    */
    fd = open(link_layer.port, O_RDWR | O_NOCTTY );
    if (fd <0) {perror(link_layer.port); exit(-1); }
    if(setTermios(fd)<0){
      perror("setting termios settings");
      return -1;
    }
    if(link_layer.status){//RECETOR
        //TODO UA protocol
        //le o set
        while (curr_level<5) {       /* loop for input */
          res = read(fd,buf,1);
          if(res>0){
            curr_level=stateMachine(buf[0],curr_level,SET);
          }
        }
        ///SEND UA
        printf("sent UA");
        res=write(fd,UA,5);
    }
    return fd;
  }
