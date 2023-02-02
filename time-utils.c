#include "time-utils.h"


void get_iso_time(char date[DATE_FORMAT_LENGTH]){
    time_t rawtime;
    time(&rawtime);
    strftime(date, DATE_FORMAT_LENGTH, "%Y-%m-%dT%H:%M:%SZ", gmtime(&rawtime));
}
