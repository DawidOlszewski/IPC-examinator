#include"stopwatch.h"


static void cleanupHandler(void* t){
    questionTime = 0;
    pthread_mutex_unlock(&time_mutex);
}

static void *startCounting(void * t){
    pthread_cleanup_push(cleanupHandler, NULL);
    pthread_mutex_lock(&time_mutex);
    questionTime = 0;
    pthread_mutex_unlock(&time_mutex);
    while(1){
        sleep(1);
        pthread_mutex_lock(&time_mutex);
        questionTime++;
        pthread_mutex_unlock(&time_mutex);

        for(int i = 0; i < MAX_PLAYER_SUPPORTED; i ++){
            if(players[i] == NULL){
                continue;
            }
            sendView(players[i], NULL);
        }
    }
    pthread_cleanup_pop(1);

    return NULL;
}

void startStopwatch(pthread_t* tid){
    check(pthread_create(tid, NULL, startCounting, NULL), "pthread create");
}

void stopStopwatch(pthread_t tid){
    check(pthread_cancel(tid), "pthread cancel");
    check(pthread_join(tid, NULL), "pthread cancel");
}

int getTime(){
    int temp;
    pthread_mutex_lock(&time_mutex);
    temp = questionTime;
    pthread_mutex_unlock(&time_mutex);
    return temp;
}