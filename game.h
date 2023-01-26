#include"player.h"

typedef enum gameState{
    NOTSTARTED,
    INPROGRESS
} gameState;

int everyPlayerFinished();
void genNewQuestion();
char* genView(Player* player);
void sendView(Player* player, char* info);
void updateScoreBoard();