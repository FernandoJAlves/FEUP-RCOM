#include "reader.h"
#include "data_link.h"
#include "stateMachine.h"
#include "application_layer.h"


unsigned char *llread(int fd, unsigned long *size)
{
	int curr_state = 0;
	int tramaNum = 0;
	unsigned char controlField;
	unsigned char c;
	unsigned char *frame = (unsigned char *)malloc(0);
	int bccCheckedData = 0;
	int destuffingError = 0;
	while (curr_state < 6)
	{
		read(fd, &c, 1);
		switch (curr_state){
		case 0:
			if (c == FLAG)
			{
				curr_state = 1;
			}
			break;
		case 1:
			if (c == FLAG)
			{
				curr_state = 1;
			}
			else if (c == Aemiss)
			{
				curr_state = 2;
			}
			else
			{
				curr_state = 0;
			}
			break;
		case 2:
		
			if (c == C0)
			{
				controlField = c;
				tramaNum = 0;
				curr_state = 3;
			}
			else if (c == C1)
			{
				controlField = c;
				tramaNum = 1;
				curr_state = 3;
			}
			else
			{
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
			if (c == FLAG)
			{
				if (checkBCC2(frame, *size))
				{
					curr_state = 6;
					bccCheckedData = 1;
					//valid bcc2
				}
				else
				{
					curr_state = 6;
					bccCheckedData = 0;
					//invalid bcc2
				}
			}
			else if (c == ESCAPEBYTE)
			{ //goes to state 5 for byte de-stuffing
				curr_state = 5;
			}
			else
			{ //reads data
				frame = (unsigned char *)realloc(frame, ++(*size));
				frame[*size - 1] = c;
			}
			break;
		case 5:
			//Byte de-stuffing
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
					printf("Non valid character after escape character\n");
					destuffingError = 1;
					//Invalid value after destuffing (caused by physical interference)
				}
			}
			curr_state = 4;
			break;
		}
	}
	frame = (unsigned char *)realloc(frame, *size-1);
	*size = *size - 1;

	printf("Trama num: %d\n", tramaNum);
	printf("Esperado: %d\n", expectedBCC);
	
	//Enviar a resposta apropriada
	if (bccCheckedData && !destuffingError)
	{
		if (tramaNum == expectedBCC)
		{
			if (tramaNum == 0)
				sendControlField(fd, RR1);
			else
				sendControlField(fd, RR0);
			printf("Enviou RR%d\n", !tramaNum);	
			expectedBCC = (expectedBCC+1)%2;
		}
		else{
			printf("ERROR: Expected %x, but trama was %x\n", expectedBCC, tramaNum);
			*size = 0;
			
			if (tramaNum == 0)
				sendControlField(fd, RR1);
			else
				sendControlField(fd, RR0);
				
		}
	}
	else{
		if(!bccCheckedData){
			printf("ERROR: Message rejected, invalid BCC\n");
		}
		*size = 0;
		if (tramaNum == 0)
			sendControlField(fd, REJ1);
		else
			sendControlField(fd, REJ0);
	}
	
	return frame;
}

int llopenR(int porta, int status)
{
	int res;
	SET[0] = FLAG;
	SET[1] = Aemiss;
	SET[2] = setC;
	SET[3] = SET[1] ^ SET[2];
	SET[4] = FLAG;

	UA[0] = FLAG;
	UA[1] = Aemiss;
	UA[2] = uaC;
	UA[3] = UA[1] ^ UA[2];
	UA[4] = FLAG;
	char buf[255];
	int curr_level = 0;
	/*
      Open serial port device for reading and writing and not as controlling tty
      because we don't want to get killed if linenoise sends CTRL-C.
    */
	fd = open(link_layer.port, O_RDWR | O_NOCTTY);
	if (fd < 0)
	{
		perror(link_layer.port);
		exit(-1);
	}
	if (setTermios(fd) < 0)
	{
		perror("Setting termios settings");
		return -1;
	}
	if (link_layer.status)
	{ //RECETOR

		while (curr_level < 5)
		{  
			res = read(fd, buf, 1);
			if (res > 0)
			{
				curr_level = stateMachine(buf[0], curr_level, SET);
			}
		}
		///SEND UA
		res = write(fd, UA, 5);
		printf("Sent UA\n");
	}
	return fd;
}


void llcloseR(int fd){

	DISCr[0]=FLAG;
	DISCr[1]=Aemiss;
	DISCr[2]=DISC; 
	DISCr[3]=DISCr[1]^DISCr[2];
	DISCr[4]=FLAG;

	UA[0] = FLAG;
	UA[1] = Aemiss;
	UA[2] = uaC;
	UA[3] = UA[1] ^ UA[2];
	UA[4] = FLAG;
	
	readControlMessageR(fd,DISCr);
	printf("Received DISC\n");
	sendControlField(fd, DISC);
	printf("Sent DISC\n");
	readControlMessageR(fd, UA);
	printf("Received UA\n");

	tcsetattr(fd, TCSANOW, &link_layer.oldPortSettings);
}

int checkBCC2(unsigned char *packet, int size)
{
	int i;
	unsigned char byte = packet[0];
	
	for (i = 1; i < size - 1; i++)
	{
		byte = byte ^ packet[i];
	}
	if (byte == packet[size - 1])
	{
		return 1;
	}
	else
		return 0;
}

