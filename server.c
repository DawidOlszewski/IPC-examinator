#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include "errors.h"
#include "server-utils.h"
#include "fd-set-utils.h"
#include "constants.h"
#include "global.h"
#include <string.h>

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

void genNewQuestion(){
    char anwsers[QUESTION_NR] = {'A', 'B', 'C', 'D'}; //TODO: its temporary
    char* questions[QUESTION_NR] = {"Q: How many eggs are in the basket?\nA: 1\nB: 2\nC: 3\nD: 4\n",
                                    "Q: How old I am?\nA: 12\nB: 18\nC: 14\nD: 13\n",
                                    "Q: Do you like her?\nA: No\nB: Yes\nC: No, I love her\nD: Braaaaa\n",
                                    "Q: Is C high level lang?\nA: Yes\nB: No\nC: Studpied question\nD: Lets waste half of lecture\n"};

    strcpy(currentQuestion, questions[question_nr-1]); 
    currentAnwser = anwsers[question_nr-1];
    question_nr++;
}

char* genView(Player* player){
    char* buffer = malloc(512 * sizeof(char));
    memset(buffer, 0, 512);
    if(player->score[question_nr-1] == -1){ //printf is null
        sprintf(buffer, "[info: %s] [time: %d]\n%s", player->lastInfo, time, currentQuestion);
        printf("generated view - question\n");
        return buffer;
    }

    sprintf(buffer, "[info: %s] [time: %d]\n%s", player->lastInfo, time, scoreBoard);
    printf("generated view - scoreBoard\n");
    return buffer;
}

void sendView(Player* player, char* info){
    if(player == NULL){
        printf("this player doesn't exist\n");
        exit(EXIT_FAILURE);
    }
    if(info[0] != '\0'){
        strcpy(player->lastInfo, info);
    }
    
    char* generatedView = genView(player);

    check(write(player->fd, generatedView, 512* sizeof(char)), "write in sendView");

    free(generatedView);
}


void updateScoreBoard(){
    char tempScoreBoard[512] = {'\0'};
    // printf("BEFORE:\n%s\n", scoreBoard);

    for(int i = 0; i < MAX_PLAYER_SUPPORTED; i++){
        if(players[i] == NULL){
            continue;
        }

        char buffer[512] = {'\0'};
        sprintf(buffer, "fd: %d :: [\n", players[i]->fd) ;
        // printf("%s\n", buffer);
        strcat(tempScoreBoard, buffer);
        memset(buffer, 0, 512); //CONT
        for(int j = 1; j <= question_nr; j++){
            if(players[i]->score[j-1] == -1){
                continue;
            }
            sprintf(buffer, "{q: %d a:%d},\n", j, players[i]->score[j-1]);
            strcat(tempScoreBoard, buffer);
            memset(buffer, 0, 512); //CONT
        }
        strcat(tempScoreBoard, "]\n");
    }
    memset(scoreBoard, 0, 512); //CONT
    strcpy(scoreBoard, tempScoreBoard);
    // printf("AFTER:\n%s\n\n", scoreBoard);
}


//TODO: send question with additional info (like previous input was corrupted pls try again)

int main()
{
    intitiaze_monitor_fd_set();
    int data_socket;
    int ret;
    char buffer[BUFFER_SIZE];
    fd_set readfds;

    int connection_socket = setup_server();
    max_fd = connection_socket;

    while(1) {
        // Copy the entire monitored FDs to readfds
        refresh_fd_set(&readfds, connection_socket);

        // Blocking system call, waiting for select call
        select(max_fd + 1, &readfds, NULL, NULL, NULL);

        printf("new msg\n");

        if(FD_ISSET(0, &readfds)){
            if(currentGameState != NOTSTARTED){
                check(read(0, buffer, 1), "read when reading from stdin when game started");
                printf("\n\nSCOREBORAD\n\n%s\n", scoreBoard);
                memset(scoreBoard, 0, 512); //TODO: dleete
                continue;
            }
            memset(buffer, 0, BUFFER_SIZE);

            ret = read(0, buffer, BUFFER_SIZE);

            if(ret == -1){
                perror("read server - from stdin");
                exit(EXIT_FAILURE);
            }

            buffer[ret] = '\0';
            
            if(strncmp(buffer, "start", 5) == 0 ){
                if(currentGameState == INPROGRESS){
                    printf("game is already in progress\n");
                    continue;
                }
                currentGameState = INPROGRESS;
                printf("game - started\n");

                //TODO: it should be wrapped in function nextQuestion()
                genNewQuestion();
                for(int i = 0; i < MAX_PLAYER_SUPPORTED; i++){
                    if(players[i] == NULL){
                        continue;
                    }
                    sendView(players[i], "game started - first question");
                }
            }else{
                printf("print \"start\" to start game\n");
            }
        }
        else if(FD_ISSET(connection_socket, &readfds)){
            // Select call to master fd, new client connection

            data_socket = handle_new_connection(connection_socket);

        }
        else // Connected client made selected call
        {
            // Find the client which has send us the call
            data_socket = get_ready_fd(&readfds);

            // Prepare the buffer to recv the data
            memset(buffer, 0, BUFFER_SIZE);

            // Blocking system call, waiting for data from client
            ret = read(data_socket, buffer, BUFFER_SIZE);
            
            // Read returns zero if socket disconnects
            if(ret == 0){
                printf("Client disconnected\n");
                remove_from_monitored_fd_set(data_socket);
                continue;
            }

            // printf("%s\n", buffer);

            if(currentGameState == NOTSTARTED){
                printf("player tried to send sth, but the game hasn't started yet\n");
                continue;
            }

            if(currentGameState == INPROGRESS){
                Player* player = get_player_by_fd(data_socket);

                if(player->score[question_nr-1] != -1){
                    printf("the player has already answered this question\n");
                    continue;
                }

                if(buffer[1] != 10 || buffer[2] != 0){
                    printf("\n");
                    for(int i = 0; i < 5; i++){
                        printf("%d ", buffer[i]);
                    }
                    printf("\n");

                    printf("the client send answer in wrong format therefore it will be ignored\n");
                    sendView(player, "wrong format of answer try again (just one letter)");
                    continue;
                }
 
                if(buffer[0] == currentAnwser){ //TODO: it should be splitted somehow
                    player->score[question_nr-1] = 1;
                    player->timeElapsed[question_nr-1] = 0; //TODO: put correct number of seconds ellapsed;
                    updateScoreBoard();
                    sendView(player, "good job");
                }else{
                    player->score[question_nr-1] = 0;
                    updateScoreBoard();
                    sendView(player, "bad anwser");
                }
                printf("updated score board : %s\n", scoreBoard);

                //send updated score board
                for(int i = 0; i < MAX_PLAYER_SUPPORTED; i++){
                    if(players[i] == NULL || players[i] == player){
                        continue;
                    }
                    if(players[i]->score[question_nr-1] != -1){
                        sendView(players[i], "new anwser arrived");
                    }
                }


                if(everyPlayerFinished() == 1){ 
                    printf("Everyone finished. Generating new question\n");
                    genNewQuestion();
                    sleep(1);
                    for(int i = 0; i < MAX_PLAYER_SUPPORTED; i++){
                        if(players[i] == NULL){
                            continue;
                        }
                        sendView(players[i], "everyone finished - new question");
                    }
                }
            }  
        }
    } 

    // Close the connection socket
    close_server(connection_socket);

    // Unlink the socket
    exit(EXIT_SUCCESS);
}