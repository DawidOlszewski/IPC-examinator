#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include"errors.h"
#include"constants.h"

int main(int argc, char* argv[])
{
    if(argc != 2){
        printf("pls pass your username as command-line argument\n");
        exit(EXIT_FAILURE);
    }

    if(strlen(argv[1]) > BUFFER_SIZE){
        printf("name is definitely too long :)\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un addr;
    int data_socket;
    int ret;
    char buffer[BUFFER_SIZE];
    fd_set read_fd_set;


    /* Create data socket. */

    data_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    if (data_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /*
     * For portability clear the whole structure, since some
     * implementations have additional (nonstandard) fields in
     * the structure.
     * */

    memset(&addr, 0, sizeof(struct sockaddr_un));

    /* Connect socket to socket address */

    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_NAME, sizeof(addr.sun_path) - 1);

    check (connect (data_socket, (const struct sockaddr *) &addr,
            sizeof(struct sockaddr_un)), "the server is down");

    check(write(data_socket, argv[1], BUFFER_SIZE), "write username");
    
    while(1){
        FD_ZERO(&read_fd_set);
        FD_SET(0, &read_fd_set);
        FD_SET(data_socket, &read_fd_set);

        
        check(select(data_socket + 1, &read_fd_set, NULL, NULL, NULL), "select");

        system("clear");  

        memset(buffer, 0, BUFFER_SIZE);

        if(FD_ISSET(0, &read_fd_set)){
            check(read(0, buffer, BUFFER_SIZE), "read client - from stdin");
            check(write(data_socket, buffer, BUFFER_SIZE), "write");
        }
        else{
            ret = check(read(data_socket, buffer, BUFFER_SIZE), "read client - from server");
            if(ret == 0){
                printf("server is down, closing\n");
                exit(EXIT_FAILURE);
            }
            printf("%s", buffer); //the \n is send also
        }
    }

    /* Close socket. */

    close(data_socket);

    exit(EXIT_SUCCESS);
}