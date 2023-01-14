#include<stdlib.h>
#include<stdio.h>

int check(int response, char* message){
    if (response == -1) {
        perror(message);
        exit(EXIT_FAILURE);
    }
    return response;
}
