#ifndef GLOBAL
#define GLOBAL

#include<pthread.h>
#include "constants.h"
#include "game.h"
#include "player.h"


extern Player* players[MAX_PLAYER_SUPPORTED];
extern gameState currentGameState;
extern int max_fd;
extern int question_nr;
extern char currentQuestion[];
extern char currentAnwser;
extern int questionTime;
extern char scoreBoard[];
extern pthread_mutex_t time_mutex;

#endif