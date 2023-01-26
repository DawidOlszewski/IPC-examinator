#ifndef playerheader
#define playerheader

typedef struct Player{
    int score[QUESTION_NR];
    int timeElapsed[QUESTION_NR];
    int fd;
    char lastInfo[128];
} Player;

#endif
