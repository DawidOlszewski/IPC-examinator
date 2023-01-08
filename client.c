#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_NAME "/tmp/DemoSocket"
#define BUFFER_SIZE 128

int
main(int argc, char *argv[])
{
    struct sockaddr_un addr;
    int i;
    int ret;
    int data_socket;
    char buffer[BUFFER_SIZE];
    int server_fd = -1;

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

    ret = connect (data_socket, (const struct sockaddr *) &addr,
            sizeof(struct sockaddr_un));

    if (ret == -1) {
        fprintf(stderr, "The server is down.\n");
        exit(EXIT_FAILURE);
    }

    fd_set read_fd_set, write_fd_set;
    while(1){

        FD_ZERO(&read_fd_set);
        FD_SET(0, &read_fd_set);
        FD_SET(data_socket, &read_fd_set);

        
        int ret = select(data_socket + 1, &read_fd_set, NULL, NULL, NULL);

        if(ret == -1){
            perror("select");
            exit(EXIT_FAILURE);
        }

        memset(buffer, 0, BUFFER_SIZE);

        if(FD_ISSET(0, &read_fd_set)){
            // printf("from input\n");
            ret = read(0, buffer, BUFFER_SIZE);
            write(data_socket, buffer, BUFFER_SIZE);
        }
        else{
            ret = read(data_socket, buffer, BUFFER_SIZE);
            printf("%s", buffer); //the \n is send also
        }
        
        if(ret == -1){
            perror("read");
            exit(EXIT_FAILURE);
        }
    }

    /* Close socket. */

    close(data_socket);

    exit(EXIT_SUCCESS);
}