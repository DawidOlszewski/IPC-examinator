#ifndef SERVER_UTILS
#define SERVER_UTILS

#include<string.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<unistd.h>
#include<stdio.h>
#include"errors.h"
#include"constants.h"
#include"global.h"


int setup_server();
int handle_new_connection(int connection_socket);
void close_server(int connection_socket);

#endif