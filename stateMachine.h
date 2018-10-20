#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_
int stateMachine(unsigned char c, int curr_state, unsigned char arg[]);
unsigned char readControlMessage(unsigned char c, int curr_state);
#endif
