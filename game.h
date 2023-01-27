#include"player.h"
#ifndef gameheader
#define gameheader
typedef enum gameState{
    NOTSTARTED,
    INPROGRESS
} gameState;

int everyPlayerFinished();
//returns 1 when all questions were used, 0 when there are more 
int genNewQuestion();
char* genView(Player* player);
void sendView(Player* player, char* info);
void updateScoreBoard();

#endif