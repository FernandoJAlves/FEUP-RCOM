#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

//Includes getip.c
#include <errno.h> 
#include <netdb.h> 
#include <sys/types.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <strings.h>

#define MAX_STRING_SIZE     128
#define SERVER_PORT 		21

// Example of input: ./run ftp://[<user>:<password>@]<host>/<url-path>
void parseInputString(char *input, char *user, char *password, char *host, char *url_path){
	
	int curr_index = 0;
	int i = 0;
	int curr_state = 0;
	int input_length = strlen(input);
    char init[] = "ftp://";

	while (i < input_length)
	{
		switch (curr_state)
		{
		case 0: //handle "ftp://"
			if (i < 5 && input[i] == init[i]) //still reading "ftp://"
				break;

			if (i == 5 && input[i] == init[i]) //finished "ftp://"
				curr_state = 1;

			else   //unexpected value
				printf("Expected 'ftp://' value\n");

			break;

		case 1: //handle user
			if (input[i] == ':'){ //finished user
				curr_state = 2;
				curr_index = 0;
			}
            else if(input[i] == '/'){ //no user, copy to host
                curr_state = 4;
				curr_index = 0;
                for(int i = 0; i < strlen(user); i++)
                    host[i] = user[i];
                memset(user, 0, MAX_STRING_SIZE);
            }
			else{ //still reading user
				user[curr_index] = input[i];
				curr_index++;
			}
			break;

		case 2: //handle password
			if (input[i] == '@'){ //finished password
				curr_state = 3;
				curr_index = 0;
			}
            else if(input[i] == '/'){ //no password, copy to host
                curr_state = 4;
				curr_index = 0;
                for(int i = 0; i < strlen(user); i++)
                    host[i] = password[i];
                memset(password, 0, MAX_STRING_SIZE);
            }
			else{ // still reading password
				password[curr_index] = input[i];
				curr_index++;
			}
			break;

		case 3: //handle host
			if (input[i] == '/'){ //finished host
				curr_state = 4;
				curr_index = 0;
			}
			else{ //still reading host
				host[curr_index] = input[i];
				curr_index++;
			}
			break;
		case 4: //handle url_path (rest of the input)
			url_path[curr_index] = input[i];
			curr_index++;
			break;
		}
		i++;
	}

}

void parseFile(char *url_path, int url_path_size, char *filename){
	
	int curr_index = 0;

	for(int i = 0; i < url_path_size; i++){

		if(url_path[i] == '/'){ // current filename is a directory, reset and start again
			curr_index = 0;
			memset(filename, 0, MAX_STRING_SIZE);
		}
		else{
			filename[curr_index] = url_path[i];
			curr_index++;
		}
	}
}

// Codigo fornecido

struct hostent* getip(char* host)
{
    struct hostent * h;
	if ((h = gethostbyname(host)) == NULL) {  
        herror("gethostbyname");
        exit(1);
    }
	return h;
}

// Fim de Codigo fornecido





int main(int argc, char** argv){

    /*Steps:
        1 - parse input
        2 - obter IP
        3 - criar socket TCP


    */

	char* user = (char*) calloc(MAX_STRING_SIZE, sizeof(char));
    char* password = (char*) calloc(MAX_STRING_SIZE, sizeof(char));
    char* host = (char*) calloc(MAX_STRING_SIZE, sizeof(char));
    char* url_path = (char*) calloc(MAX_STRING_SIZE, sizeof(char));

    parseInputString(argv[1], user, password, host, url_path);

    char* filename = (char*) calloc(MAX_STRING_SIZE, sizeof(char));

    parseFile(url_path, strlen(url_path), filename);

    //Testing the values -> Tested and works fine
    printf("\nUser: %s\n", user);
    printf("Password: %s\n", password);
    printf("Host: %s\n", host);
    printf("URL Path: %s\n", url_path);
    printf("File name: %s\n\n", filename);

    struct hostent* h = getip(host);

    printf("Host name  : %s\n", h->h_name);
    printf("IP Address : %s\n\n",inet_ntoa(*((struct in_addr *)h->h_addr)));

	// Codigo fornecido 

	int	sockfd;
	struct	sockaddr_in server_addr;
	char buf[] = "Mensagem de teste na travessia da pilha TCP/IP\n";  
	int	bytes;

	/*server address handling*/
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)h->h_addr)));	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(SERVER_PORT);		/*server TCP port must be network byte ordered */
    
	/*open an TCP socket*/
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
    	perror("socket()");
        exit(0);
    }

	/*connect to the server*/
    if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("connect()");
		exit(0);
	}

    /*send a string to the server*/
	bytes = write(sockfd, buf, strlen(buf));
	printf("Bytes escritos %d\n", bytes);



    // Freeing variables

    free(user);
    free(password);
    free(host);
    free(url_path);
    
    return 0;
}

