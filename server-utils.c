#include<string.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<unistd.h>
#include"fd-set-utils.h"
#include"errors.h"
#include"constants.h"
#include"fd-set-utils.h"

int setup_server(int * monitored_fd_set){
    struct sockaddr_un name; // Socket type and socket name
    int connection_socket;
    // In case some process is already listening on our socket name
    unlink(SOCKET_NAME);

    // create master file descriptor in stream type connection 
    connection_socket = check(
            socket(AF_UNIX, SOCK_STREAM, 0), 
                "creating connection socket failed");

    // Initializing connection socket struct
    memset(&name, 0, sizeof(struct sockaddr_un));

    // Providing socket family and socket name
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);

    // Bind socket to socket name, every request sent to socket name will be redirected to our server 
    check(
        bind(connection_socket, (const struct sockaddr *) &name, sizeof(struct sockaddr_un)),
            "bind failed");

    check(
        listen(connection_socket, MAX_CLIENT_SUPPORTED), 
            "listen failed");

    add_to_monitored_fd_set(monitored_fd_set, connection_socket);

    printf("server.o\n");

    return connection_socket;
}

void close_server(int connection_socket){
    close(connection_socket);
    unlink(SOCKET_NAME);
    printf("Connection closed..\n");
}

int add_client(int* monitored_fd_set, int connection_socket){
    // Master socket call, new client connects
    printf("New connection recieved, accept the connection\n");

    int data_socket = check(accept(connection_socket, NULL, NULL), 
                    "accept failed");

    printf("Connection accepted from client\n");

    add_to_monitored_fd_set(monitored_fd_set, data_socket);

    return data_socket;
}

