#include<sys/select.h>
#include"constants.h"
#include"global.h"
#include<stdio.h> 
#define max(x, y) x > y ? x : y


// Remove all the FDs, if any, from the the array
void intitiaze_monitor_fd_set(){     
    for(int i = 0 ; i < MAX_CLIENT_SUPPORTED; i++){
        monitored_fd_set[i] = -1;
    }
}

// Add a new FD to the monitored_fd_set array
void add_to_monitored_fd_set(int skt_fd){   
    for(int i = 0; i < MAX_CLIENT_SUPPORTED; i++){
        if(monitored_fd_set[i] != -1){
            continue;
        }
        monitored_fd_set[i] = skt_fd;
        max_fd = max(skt_fd, max_fd);
        return;
    }
    printf("cannot add another client because max limit is exceeded\n");
}

// Remove the FD from monitored_fd_set array
void remove_from_monitored_fd_set(int skt_fd){
    for(int i = 0; i < MAX_CLIENT_SUPPORTED; i++){
        if(monitored_fd_set[i] != skt_fd)
            continue;
        monitored_fd_set[i] = -1;
        break;
    }
}

// Copy monitored fds to fd_set
void refresh_fd_set(fd_set *fd_set_ptr){
    FD_ZERO(fd_set_ptr);
    for(int i = 0; i < MAX_CLIENT_SUPPORTED; i++){
        if(monitored_fd_set[i] != -1){
            FD_SET(monitored_fd_set[i], fd_set_ptr);
        }
    }
}

int get_ready_fd(fd_set* readfds){
    for(int i = 0; i < MAX_CLIENT_SUPPORTED; i++){
        if(FD_ISSET(i, readfds)){
            return i;
        }
    }
}
