#include<sys/select.h>
#include"constants.h"
#include"player.h"

void intitiaze_monitor_fd_set();
void add_to_monitored_fd_set(int skt_fd);
void remove_from_monitored_fd_set(int skt_fd);
void refresh_fd_set(fd_set *fd_set_ptr, int connectionSocket);
int get_ready_fd(fd_set* readfds);
Player* get_player_by_fd(int fd);
