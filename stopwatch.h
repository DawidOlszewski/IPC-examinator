#ifndef STOPWATCH
#define STOPWATCH

#include<pthread.h>
#include<unistd.h>
#include"errors.h"
#include"global.h"

void startStopwatch(pthread_t* tid);
void stopStopwatch(pthread_t tid);
int getTime();

#endif