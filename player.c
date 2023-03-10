#include "player.h"

#define max(x, y) x > y ? x : y


void intitiazePlayers(){     
    for(int i = 0 ; i < MAX_PLAYER_SUPPORTED; i++){
        players[i] = NULL;
    }
}

void createPlayer(int skt_fd, char* username){   
    for(int i = 0; i < MAX_PLAYER_SUPPORTED; i++){
        if(players[i] != NULL){
            continue;
        }
        players[i] = malloc(sizeof(Player));
        players[i] -> fd = skt_fd;

        for(int j = 0; j < QUESTION_NR; j++){
            players[i]->score[j] = -1;
            players[i]->timeElapsed[j] = -1;
            strncpy(players[i]->lastInfo, "", 127);
            strncpy(players[i]->username, username, 127);
        }
        max_fd = max(skt_fd, max_fd);
        return;
    }
    printf("cannot add another player because max limit is exceeded\n");
}

// Remove the FD from players array
void removePlayer(Player* player){
    int removed = 0;
    for(int i = 0; i < MAX_PLAYER_SUPPORTED; i++){
        if(players[i] != NULL){
            max_fd = max(max_fd, players[i]->fd);
        }
        if(players[i] == player){
            free(players[i]);
            players[i] = NULL;
            removed = 1;
        }
    }
    if(removed == 0){
        printf("Error: this player doesn't exist.\n");
        exit(EXIT_FAILURE);
    }
}

// Copy monitored fds to fd_set
void getPlayersFds(fd_set *fd_set_ptr, int connectionSocket){
    FD_ZERO(fd_set_ptr);
    FD_SET(0, fd_set_ptr);
    FD_SET(connectionSocket, fd_set_ptr);

    for(int i = 0; i < MAX_PLAYER_SUPPORTED; i++){
        if(players[i] != NULL){
            FD_SET(players[i]->fd, fd_set_ptr);
        }
    }
}

int getReadyFd(fd_set* readfds){
    for(int i = 0; i < MAX_PLAYER_SUPPORTED; i++){
        if(FD_ISSET(i, readfds)){
            return i;
        }
    }
    printf("cannot get fd\n");
    exit(EXIT_FAILURE);
}


Player* getPlayerByFd(int fd){
    for(int i = 0; i < MAX_PLAYER_SUPPORTED; i++){
        if(players[i] != NULL && players[i]->fd == fd){
            return players[i];
        }
    }
    printf("cannot found player with this fd set\n");
    exit(EXIT_FAILURE);
}

Player* getReadyPlayer(fd_set* readfds){
    int fd = getReadyFd(readfds);
    return getPlayerByFd(fd);
}

int isPlayerArrayEmpty(){
    for(int i = 0; i < MAX_PLAYER_SUPPORTED; i++){
        if(players[i] != NULL){
            return 0;
        }
    }
    return 1;
}