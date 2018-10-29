#include <sys/time.h>
#include "utilities.h"
#include <stddef.h>


struct timeval start;
struct timeval t;

void startCounter(){
    gettimeofday(&start,NULL);
}

double getTransferRate(int numBytes){
    gettimeofday(&t,NULL);
    double time = ((double) t.tv_sec - start.tv_sec) + ((double)t.tv_usec - start.tv_usec)/1000/1000;
    return (numBytes/time/1000.0);
}