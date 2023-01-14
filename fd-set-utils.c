#include<sys/select.h>
#include"constants.h"

// Remove all the FDs, if any, from the the array
void intitiaze_monitor_fd_set(int* monitored_fd_set){     
    for(int i = 0 ; i < MAX_CLIENT_SUPPORTED; i++){
        monitored_fd_set[i] = -1;
    }
}

// Add a new FD to the monitored_fd_set array
void add_to_monitored_fd_set(int* monitored_fd_set, int skt_fd){     
    for(int i = 0; i < MAX_CLIENT_SUPPORTED; i++){
        if(monitored_fd_set[i] != -1)
            continue;
        monitored_fd_set[i] = skt_fd;
        break;
    }
}

// Remove the FD from monitored_fd_set array
void remove_from_monitored_fd_set(int* monitored_fd_set, int skt_fd){
    for(int i = 0; i < MAX_CLIENT_SUPPORTED; i++){
        if(monitored_fd_set[i] != skt_fd)
            continue;
        monitored_fd_set[i] = -1;
        break;
    }
}

// Copy monitored fds to fd_set
void refresh_fd_set(int* monitored_fd_set, fd_set *fd_set_ptr){

    FD_ZERO(fd_set_ptr);
    for(int i = 0; i < MAX_CLIENT_SUPPORTED; i++){
        if(monitored_fd_set[i] != -1){
            FD_SET(monitored_fd_set[i], fd_set_ptr);
        }
    }
}

int get_ready_fd(fd_set* readfds, int max_fd){
    for(int i = 0; i < max_fd; i++){
        if(FD_ISSET(i, readfds)){
            return i;
        }
    }
}
