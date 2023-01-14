#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include "errors.h"
#include "server-utils.h"
#include "fd-set-utils.h"
#include "constants.h"


#define max(x, y) x > y ? x : y

int main(int argc, char *argv[])
{
    int data_socket;
    int result;
    int data;
    int ret;
    char buffer[BUFFER_SIZE];
    int monitored_fd_set[MAX_CLIENT_SUPPORTED];
    fd_set readfds;

    intitiaze_monitor_fd_set(monitored_fd_set);

    int connection_socket = setup_server(monitored_fd_set);
    int max_fd = connection_socket;

    while(1) {
        // Copy the entire monitored FDs to readfds
        refresh_fd_set(monitored_fd_set, &readfds);

        // Blocking system call, waiting for select call
        select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if(FD_ISSET(connection_socket, &readfds)){
            // Select call to master fd, new client connection
            data_socket = add_client(monitored_fd_set, connection_socket);

            // Keep track of biggest fd
            max_fd = max(max_fd, data_socket);            
        }
        else // Connected client made selected call
        {
            // Find the client which has send us the call
            data_socket = get_ready_fd(&readfds, max_fd);

            // Prepare the buffer to recv the data
            memset(buffer, 0, BUFFER_SIZE);

            // Blocking system call, waiting for data from client
            ret = read(data_socket, buffer, BUFFER_SIZE);
            
            // Read returns zero if socket disconnects
            if(ret == 0){
                printf("Client disconnected\n");
                remove_from_monitored_fd_set(monitored_fd_set, data_socket);
                continue;
            }

            for(int i = 0; i < max_fd; i++){
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