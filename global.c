#include "constants.h"
#include "game.h"
#include "fd-set-utils.h"

int monitored_fd_set[MAX_CLIENT_SUPPORTED];
gameState currentGameState = NOTSTARTED;
int max_fd;