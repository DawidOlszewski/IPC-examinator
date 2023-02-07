#include "game.h"

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
    Question* parsed_question = get_question(question_nr+1); 

    char formatted_question[BUFFER_SIZE];

    sprintf(formatted_question, "\n%s\n%c: %s%c: %s%c: %s%c: %s\n", 
    parsed_question->question_content,
    parsed_question->answers[0].identifier, parsed_question->answers[0].answer_content,
    parsed_question->answers[1].identifier, parsed_question->answers[1].answer_content,
    parsed_question->answers[2].identifier, parsed_question->answers[2].answer_content,
    parsed_question->answers[3].identifier, parsed_question->answers[3].answer_content);
    
    strcpy(currentQuestion, formatted_question);
     
    currentAnwser = parsed_question->answers[parsed_question->correct_answer-1].identifier;
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
        sprintf(buffer, "%s\n", players[i]->username) ;
        strcat(tempScoreBoard, buffer);
        memset(buffer, 0, 512); 
        for(int j = 1; j <= question_nr; j++){
            if(players[i]->score[j-1] == -1){
                continue;
            }
            sprintf(buffer, "\t%d -> %s (time: %d),\n", j, players[i]->score[j-1] == 1 ? "Correct" : "Incorrect", players[i]->timeElapsed[j-1]);
            strcat(tempScoreBoard, buffer);
            memset(buffer, 0, 512); 
        }
    }
    memset(scoreBoard, 0, 512); 
    strcpy(scoreBoard, tempScoreBoard);
}

void sendFinalScoreboard(Player *player)
{
    char formatted_time[DATE_FORMAT_LENGTH];
    get_iso_time(formatted_time);
    char *buffer = calloc(512, sizeof(char));
    sprintf(buffer, "Game finished on %s\nFinal score: \n%s", formatted_time, scoreBoard);
    check(write(player->fd, buffer, 512 * sizeof(char)), "write in sendFinalScoreboard");
    free(buffer);
}

void send_score_to_players() {
    printf("Sending final scoreboard to players.\n");
    for (int i = 0; i < MAX_PLAYER_SUPPORTED; i++)
    {
        if (players[i] == NULL)
            continue;
        sendFinalScoreboard(players[i]);
    }
}