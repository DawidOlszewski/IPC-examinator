#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include "errors.h"
#include "server-utils.h"
#include "fd-set-utils.h"
#include "constants.h"
#include "global.h"

int main(int argc, char *argv[])
{
    intitiaze_monitor_fd_set();
    int data_socket;
    int ret;
    char buffer[BUFFER_SIZE];
    fd_set readfds;

    add_to_monitored_fd_set(0); //stdin 

    int connection_socket = setup_server();
    max_fd = connection_socket;

    while(1) {
        // Copy the entire monitored FDs to readfds
        refresh_fd_set(&readfds);

        // Blocking system call, waiting for select call
        select(max_fd + 1, &readfds, NULL, NULL, NULL);

        printf("new msg\n");

        if(FD_ISSET(0, &readfds)){
            memset(buffer, 0, BUFFER_SIZE);

            ret = read(0, buffer, BUFFER_SIZE);

            if(ret == -1){
                perror("read server - from stdin");
                exit(EXIT_FAILURE);
            }

            buffer[ret] = '\0';
            
            if(strncmp(buffer, "start", 5) == 0 ){
                printf("game - started\n");
            }else{
                printf("print \"start\" to start game\n");
            }
        }
        else if(FD_ISSET(connection_socket, &readfds)){
            // Select call to master fd, new client connection

            data_socket = handle_new_connection(connection_socket);

        }
        else // Connected client made selected call
        {
            // Find the client which has send us the call
            data_socket = get_ready_fd(&readfds);

            // Prepare the buffer to recv the data
            memset(buffer, 0, BUFFER_SIZE);

            // Blocking system call, waiting for data from client
            ret = read(data_socket, buffer, BUFFER_SIZE);
            
            // Read returns zero if socket disconnects
            if(ret == 0){
                printf("Client disconnected\n");
                remove_from_monitored_fd_set(data_socket);
                continue;
            }

            for(int i = 0; i < MAX_CLIENT_SUPPORTED; i++){
                // Send data to every client except master and data sender
                if(monitored_fd_set[i] == -1 || monitored_fd_set[i] == connection_socket || monitored_fd_set[i] == data_socket)
                    continue;

                check(
                    write(monitored_fd_set[i], buffer, BUFFER_SIZE),
                        "write to recv failed");
            }            
            
        }
    } 

    // Close the connection socket
    close_server(connection_socket);

    // Unlink the socket
    exit(EXIT_SUCCESS);
}