#include<string.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<unistd.h>
#include"fd-set-utils.h"
#include"errors.h"
#include"constants.h"
#include"global.h"

int setup_server(){
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
        listen(connection_socket, MAX_PLAYER_SUPPORTED), 
            "listen failed");

    add_to_monitored_fd_set(connection_socket);

    printf("server is listening...\n");

    return connection_socket;
}

void close_server(int connection_socket){
    close(connection_socket);
    unlink(SOCKET_NAME);
    printf("Connection closed..\n");
}

int handle_new_connection(int connection_socket){
    // Master socket call, new client connects
    printf("New connection recieved\n");

    int data_socket = check(accept(connection_socket, NULL, NULL), 
                    "accept failed");

    if(currentGameState == NOTSTARTED){
        printf("Connection accepted from client\n");
        add_to_monitored_fd_set(data_socket);
    }else{
        printf("connection closed because game have already started\n");
        check(close(data_socket), "close new data_socket"); //if we want to clase we have to accept it first
    }

    return data_socket;
}

