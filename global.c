#include "global.h"

Player* players[MAX_PLAYER_SUPPORTED];// has to be initialized
gameState currentGameState = NOTSTARTED;
int max_fd;
int question_nr = 0;
char currentQuestion[1024];
char scoreBoard[1024];
char currentAnwser;
int questionTime = 0;
pthread_mutex_t time_mutex;
