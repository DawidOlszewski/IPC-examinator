#include "time-utils.h"

char* get_iso_time(){
    time_t rawtime;
    char* buff = malloc(80*sizeof(char));
    time(&rawtime);
    strftime(buff, 80, "%Y-%m-%dT%H:%M:%SZ", gmtime(&rawtime));
    return buff;
    
}
