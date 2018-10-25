#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_
int stateMachine(unsigned char c, int curr_state, unsigned char arg[]);

unsigned char readControlMessage(int fd,unsigned char * ctrl,int flag);
unsigned char SET[5];
unsigned char UA[5];
unsigned char DISCw[5];
unsigned char DISCr[5];
unsigned char RRv[5];

#define WMODE 1
#define RMODE 0

#endif
