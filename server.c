#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include "errors.h"
#include "server-utils.h"
#include "player.h"
#include "constants.h"
#include "global.h"
#include "stopwatch.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>
#include <sys/wait.h>

#define SOCKET_NAME "/tmp/DemoSocket"
#define BUFFER_SIZE 128

#define PATH_LENGTH 64
#define QUESTION_PATH "./questions/"

#define MAX_CLIENT_SUPPORTED 32

#define max(x, y) x > y ? x : y

typedef struct Answer
{
    char identifier;
    char answer_content[BUFFER_SIZE];
} Answer;

typedef struct Question
{
    int id;
    char question_content[BUFFER_SIZE];
    Answer answers[4];
    int correct_answer;
} Question;


void path_from_id(char *path, int id)
{
    // Convert id to string
    int num_length = (int)((ceil(log10(id)) + 1));
    char id_repr[num_length];
    sprintf(id_repr, "%d", id);

    strcat(path, QUESTION_PATH);
    strcat(path, id_repr);
    strcat(path, ".txt");
}

Question *allocate_shared_memory(const char *name)
{
    int shared_memory_fd;
    int shared_memory_size;

    shared_memory_size = sizeof(Question);
    
    // In case of unexpected error in parser clear shared memory
    shm_unlink(name);

    shared_memory_fd = shm_open(name, O_CREAT | O_EXCL | O_RDWR, S_IRWXU | S_IRWXG);
    if (shared_memory_fd < 0)
    {
        perror("Error allocating shared memory");
        exit(EXIT_FAILURE);
    }

    printf("Created shared memory %s\n", name);

    // Expand shared memory to shared_memory_size
    ftruncate(shared_memory_fd, shared_memory_size);

    // Map parsed question to shared memory
    Question* parsed_question = (Question *)mmap(NULL, shared_memory_size, PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_fd, 0);

    // Close shared memory fd
    close(shared_memory_fd);

    return parsed_question;
}

void parse_question_from_file(char *path, Question *parsed_question, int question_id)
{
    FILE *ptr = fopen(path, "r");
    if (ptr == NULL)
    {
        printf("No such file");
        exit(EXIT_FAILURE);
    }

    // get question content
    char buf[BUFFER_SIZE];
    fgets(buf, BUFFER_SIZE, ptr);
    strcpy(parsed_question->question_content, buf);

    // get all answers
    for (int i = 0; i < 4; i++)
    {
        fgets(buf, BUFFER_SIZE, ptr);
        parsed_question->answers[i].identifier = buf[0];

        fgets(buf, BUFFER_SIZE, ptr);
        strcpy(parsed_question->answers[i].answer_content, buf);
    }

    // get correct answer index
    fgets(buf, BUFFER_SIZE, ptr);
    parsed_question->correct_answer = atoi(buf);
    
    parsed_question->id = question_id;
    free(ptr);
}

Question *get_question(int question_id)
{
    const char *name = "QUESTION_OBJECT";

    Question *parsed_question = allocate_shared_memory(name);

    if (parsed_question == NULL)
    {
        perror("Error while mapping");
        exit(EXIT_FAILURE);
    }

    pid_t childPID = fork();
    int status;

    if (childPID == -1)
    {
        perror("Error while forking");
        exit(EXIT_FAILURE);
    }
    if (childPID == 0)
    {

        char *path = (char *)malloc(sizeof(char) * PATH_LENGTH);
        path_from_id(path, question_id);

        parse_question_from_file(path, parsed_question, question_id);

        exit(0);
    }
    else
    {
        // wait until the child process finished
        wait(&status);

        // detach the shared memory segment
        shm_unlink(name);
        return parsed_question;
    }
}

int main()
{
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

        if(FD_ISSET(0, &readfds)){
            //when the game is in progress but you wanna check store it server terminal
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
        else if(FD_ISSET(connection_socket, &readfds)){
            // Select call to master fd, new client connection
            handle_new_connection(connection_socket);
        }
        else // Connected client made selected call
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
 
                if(buffer[0] == currentAnwser){ //TODO: it should be splitted somehow
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

    //TODO: send the score board to players;


    // Close the connection socket
    close_server(connection_socket);

    // Unlink the socket
    exit(EXIT_SUCCESS);
}