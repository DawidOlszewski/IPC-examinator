#include<pthread.h>

void startStopwatch(pthread_t* tid);
void stopStopwatch(pthread_t tid);
int getTime();