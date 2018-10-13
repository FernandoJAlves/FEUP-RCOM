#ifndef APPLICATION_LAYER_H_
#define APPLICATION_LAYER_H_

typedef struct {
int fileDescriptor; /*Descritor correspondente à porta série*/
int status; /*TRANSMITTER | RECEIVER*/
}applicationLayer;

applicationLayer app_layer;
#endif
