#include "constants.h"
#include "game.h"
#include "player.h"

Player* players[MAX_PLAYER_SUPPORTED];// has to be initialized
gameState currentGameState = NOTSTARTED;
int max_fd;