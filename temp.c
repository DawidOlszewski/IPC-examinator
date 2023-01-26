#include<string.h>
#include<stdio.h>

char global[1024] = {};

int main(){
    char g[] = "badsf";
    strcpy(global, g);
    printf("%s\n", global);
}   