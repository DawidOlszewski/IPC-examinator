#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include <unistd.h>
#include"constants.h"
#include"global.h"
#include"player.h"
#include"errors.h"
#include"game.h"
#include"stopwatch.h"

int everyPlayerFinished(){
    for(int i = 0; i< MAX_PLAYER_SUPPORTED; i++){
        if(players[i] == NULL){
            continue;
        }
        if(players[i]->score[question_nr-1] == -1){
            return 0;
        }
    }
    return 1;
}

int genNewQuestion(){
    if(question_nr == QUESTION_NR){
        return 1;
    }
    char anwsers[QUESTION_NR] = {'A', 'B', 'C', 'D'}; //TODO: its temporary
    char* questions[QUESTION_NR] = {"Q: How many eggs are in the basket?\nA: 1\nB: 2\nC: 3\nD: 4\n",
                                    "Q: How old I am?\nA: 12\nB: 18\nC: 14\nD: 13\n",
                                    "Q: Do you like her?\nA: No\nB: Yes\nC: No, I love her\nD: Braaaaa\n",
                                    "Q: Is C high level lang?\nA: Yes\nB: No\nC: Studpied question\nD: Lets waste half of lecture\n"};

    strcpy(currentQuestion, questions[question_nr]); 
    currentAnwser = anwsers[question_nr];
    question_nr++;
    return 0;
}

char* genView(Player* player){
    char* buffer = malloc(512 * sizeof(char));
    memset(buffer, 0, 512);
    if(player->score[question_nr-1] == -1){ 
        sprintf(buffer, "[info: %s] [time: %d]\n%s", player->lastInfo, getTime(), currentQuestion);
        printf("generated view - question\n");
        return buffer;
    }

    sprintf(buffer, "[info: %s] [time: %d]\n%s", player->lastInfo, getTime(), scoreBoard);
    printf("generated view - scoreBoard\n");
    return buffer;
}

void sendView(Player* player, char* info){
    if(player == NULL){
        printf("this player doesn't exist\n");
        exit(EXIT_FAILURE);
    }
    if(info != NULL){
        strcpy(player->lastInfo, info);
    }
    
    char* generatedView = genView(player);

    check(write(player->fd, generatedView, 512* sizeof(char)), "write in sendView");

    free(generatedView);
}


void updateScoreBoard(){
    char tempScoreBoard[512] = {'\0'};
    for(int i = 0; i < MAX_PLAYER_SUPPORTED; i++){
        if(players[i] == NULL){
            continue;
        }

        char buffer[512] = {'\0'};
        sprintf(buffer, "fd: %d :: [\n", players[i]->fd) ;
        strcat(tempScoreBoard, buffer);
        memset(buffer, 0, 512); 
        for(int j = 1; j <= question_nr; j++){
            if(players[i]->score[j-1] == -1){
                continue;
            }
            sprintf(buffer, "%d -> %s (time: %d),\n", j, players[i]->score[j-1] == 1 ? "Correct" : "Incorrect", players[i]->timeElapsed[j-1]);
            strcat(tempScoreBoard, buffer);
            memset(buffer, 0, 512); 
        }
        strcat(tempScoreBoard, "]\n");
    }
    memset(scoreBoard, 0, 512); 
    strcpy(scoreBoard, tempScoreBoard);
}