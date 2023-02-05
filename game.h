#ifndef gameheader
#define gameheader

#include"player.h"
#include "time-utils.h"
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include <unistd.h>
#include"constants.h"
#include"player.h"
#include"errors.h"
#include"stopwatch.h"

typedef enum gameState{
    NOTSTARTED,
    INPROGRESS
} gameState;

#include"global.h"
#include "file-access.h"

int everyPlayerFinished();
//returns 1 when all questions were used, 0 when there are more 
int genNewQuestion();
char* genView(Player* player);
void sendView(Player* player, char* info);
void updateScoreBoard();
void sendFinalScoreboard(Player* player);
void send_score_to_players();
#endif