#ifndef APPLICATION_LAYER_H_
#define APPLICATION_LAYER_H_

typedef struct {
int fileDescriptor; /*Descritor correspondente à porta série*/
int status; /*TRANSMITTER | RECEIVER*/
}applicationLayer;

applicationLayer app_layer;

unsigned char * makeControlPackage_I(off_t fileSize, unsigned char * fileName, int fileName_size, int * finalSize, unsigned char start_or_end);
unsigned char * readFile(unsigned char * fileName, long int * fileSize);
void data_writer(int argc, char * argv[]);
void data_reader(int argc, char * argv[]);

#endif
