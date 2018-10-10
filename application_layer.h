#define TRANSMITTER 0;
#define RECEIVER 1;

typedef struct {
int fileDescriptor; /*Descritor correspondente à porta série*/
int status; /*TRANSMITTER | RECEIVER*/
}applicationLayer;

applicationLayer app_layer;

