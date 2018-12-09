#include <sys/time.h>
#include "utilities.h"
#include <stddef.h>


struct timeval start;
struct timeval t;

struct timeval initial;
struct timeval finalTime;

void initCounter(){
    gettimeofday(&initial,NULL);
}

void startCounter(){
    gettimeofday(&start,NULL);
}

double getTransferRate(int numBytes){
    gettimeofday(&t,NULL);
    double time = ((double) t.tv_sec - start.tv_sec) + ((double)t.tv_usec - start.tv_usec)/1000/1000;
    return (numBytes/time/1000.0);
}

double getDeltaTime(){
    gettimeofday(&finalTime,NULL);
    double time = ((double) finalTime.tv_sec - initial.tv_sec) + ((double)finalTime.tv_usec - initial.tv_usec)/1000/1000;
    return time;
}

