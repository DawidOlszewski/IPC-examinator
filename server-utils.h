int setup_server(int * monitored_fd_set);
int add_client(int* monitored_fd_set, int connection_socket);
int close_server(int connection_socket);