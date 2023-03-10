#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include "server-utils.h"


int main()
{
    int count = count_question_files();
    if(count < QUESTION_NR) {
        fprintf(stderr, "%d questions expected, found only %d", QUESTION_NR, count);
        exit(EXIT_FAILURE);
    }

    intitiazePlayers();
    int ret;
    char buffer[BUFFER_SIZE];
    fd_set readfds;
    pthread_t time_thread;

    int connection_socket = setup_server();

    max_fd = connection_socket;

    while(1) {
        // Copy the entire monitored FDs to readfds

        getPlayersFds(&readfds, connection_socket);

        // Blocking system call, waiting for select call
        select(max_fd + 1, &readfds, NULL, NULL, NULL);

        printf("new msg arrived\n");
        //from stdio
        if(FD_ISSET(0, &readfds)){
            //when the game is in progress but you wanna check store it server terminal
            if(currentGameState != NOTSTARTED){
                check(read(0, buffer, 1), "read when reading from stdin when game started");
                printf("\n\nSCOREBORAD\n\n%s\n", scoreBoard);
                memset(scoreBoard, 0, 512);
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

                if(isPlayerArrayEmpty()){
                    printf("there are not any players at that moment, therfore game cannot be started\n");
                    continue;
                }
                currentGameState = INPROGRESS;
                printf("game - started\n");

                startStopwatch(&time_thread);
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
        //new connection
        else if(FD_ISSET(connection_socket, &readfds)){
            // Select call to master fd, new client connection
            handle_new_connection(connection_socket);
        }
        else // Connected client msg
        {
            // Find the player which has send us the call
            Player* player = getReadyPlayer(&readfds);

            // Prepare the buffer to recv the data
            memset(buffer, 0, BUFFER_SIZE);

            // Blocking system call, waiting for data from client

            ret = read(player->fd, buffer, BUFFER_SIZE);
            
            // Read returns zero if socket disconnects
            if (ret == 0)
            {
                printf("Client disconnected\n");
                removePlayer(player);
                if(isPlayerArrayEmpty() == 1 && currentGameState == INPROGRESS){
                    printf("All players finished gameplay, therefore server goes down\n");
                    exit(EXIT_SUCCESS);
                }
                continue;
            }

            if(currentGameState == NOTSTARTED){
                printf("player tried to send sth, but the game hasn't started yet\n");
                continue;
            }

            if(currentGameState == INPROGRESS){

                if(player->score[question_nr-1] != -1){
                    printf("the player has already answered this question\n");
                    continue;
                }

                //when the buffer is corrupted we read \n \0 from stdin in client
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
 
                //checking anwser
                if(buffer[0] == currentAnwser){
                    player->score[question_nr-1] = 1;
                    player->timeElapsed[question_nr-1] = getTime(); 
                    updateScoreBoard();
                    sendView(player, "good job");
                }else{
                    player->score[question_nr-1] = 0;
                    player->timeElapsed[question_nr-1] = getTime(); 
                    updateScoreBoard();
                    sendView(player, "bad anwser");
                }

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
                    printf("Everyone finished.\n");
                    ret = genNewQuestion();
                    stopStopwatch(time_thread);
                    sleep(3);
                    startStopwatch(&time_thread);
                    if(ret == 1){
                        break;
                    }
                    for(int i = 0; i < MAX_PLAYER_SUPPORTED; i++){
                    printf("Generating new question.\n");
                        if(players[i] == NULL){
                            continue;
                        }
                        sendView(players[i], "everyone finished - new question");
                    }
                }
            }  

        }
    }

    stopStopwatch(time_thread);


    send_score_to_players();

    save_final_scoreboard();
    
    sleep(10);
    
    // Close the connection socket
    close_server(connection_socket);


    // Unlink the socket
    exit(EXIT_SUCCESS);
}