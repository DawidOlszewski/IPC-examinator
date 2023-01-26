#include<sys/select.h>
#include"constants.h"
#include"global.h"
#include<stdio.h> 
#include<stdlib.h>
#include<string.h>
#define max(x, y) x > y ? x : y


void intitiaze_monitor_fd_set(){     
    for(int i = 0 ; i < MAX_PLAYER_SUPPORTED; i++){
        players[i] = NULL;
    }
}

void add_to_monitored_fd_set(int skt_fd){   
    for(int i = 0; i < MAX_PLAYER_SUPPORTED; i++){
        if(players[i] != NULL){
            continue;
        }
        players[i] = malloc(sizeof(Player));
        players[i] -> fd = skt_fd;

        for(int j =0; j < QUESTION_NR; j++){
            players[i]->score[j] = -1;
            players[i]->timeElapsed[j] = -1;
            strcpy(players[i]->lastInfo, "");
        }
        max_fd = max(skt_fd, max_fd);
        return;
    }
    printf("cannot add another player because max limit is exceeded\n");
}

// Remove the FD from players[ array
void remove_from_monitored_fd_set(int skt_fd){
    for(int i = 0; i < MAX_PLAYER_SUPPORTED; i++){
        if(players[i] != NULL || players[i]->fd != skt_fd)
            continue;
        free(players[i]);
        players[i] = NULL;
        break;
    }
}

// Copy monitored fds to fd_set
void refresh_fd_set(fd_set *fd_set_ptr, int connectionSocket){
    FD_ZERO(fd_set_ptr);
    FD_SET(0, fd_set_ptr);
    FD_SET(connectionSocket, fd_set_ptr);

    for(int i = 0; i < MAX_PLAYER_SUPPORTED; i++){
        if(players[i] != NULL){
            FD_SET(players[i]->fd, fd_set_ptr);
        }
    }
}

int get_ready_fd(fd_set* readfds){
    for(int i = 0; i < MAX_PLAYER_SUPPORTED; i++){
        if(FD_ISSET(i, readfds)){
            return i;
        }
    }
    printf("cannot get fd\n");
    exit(EXIT_FAILURE);
}


Player* get_player_by_fd(int fd){
    for(int i = 0; i < MAX_PLAYER_SUPPORTED; i++){
        if(players[i] != NULL && players[i]->fd == fd){
            return players[i];
        }
    }
    printf("cannot found player with this fd set\n");
    exit(EXIT_FAILURE);
}