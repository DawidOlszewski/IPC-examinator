#ifndef playerheader
#define playerheader

#include<sys/select.h>
#include"constants.h"
#include<stdio.h> 
#include<stdlib.h>
#include<string.h>

typedef struct Player{
    int score[QUESTION_NR];
    int timeElapsed[QUESTION_NR];
    int fd;
    char username[128];
    char lastInfo[128];
} Player;

#include"global.h"

void intitiazePlayers();
void createPlayer(int skt_fd, char* username);
void removePlayer(Player* player);
void getPlayersFds(fd_set *fd_set_ptr, int connectionSocket);
//1 - There are not players
//0 - At least one player is connected
int isPlayerArrayEmpty();
int getReadyFd(fd_set* readfds);
Player* getReadyPlayer(fd_set* readfds);

#endif
