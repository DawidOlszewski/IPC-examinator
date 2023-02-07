#ifndef FILE_ACCESS
#define FILE_ACCESS

#include "constants.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "global.h"
#include <dirent.h>

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

void create_path_from_id(char *path, int id);

Question *allocate_shared_memory(const char *name);

void parse_question_from_file(char *path, Question *parsed_question, int question_id);

Question *get_question(int question_id);

void save_final_scoreboard();

int count_question_files();

void verify_question_count();

#endif