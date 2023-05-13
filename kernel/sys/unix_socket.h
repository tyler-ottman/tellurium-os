#ifndef UNIX_SOCKET
#define UNIX_SOCKET

#include <sys/socket.h>

int unix_socket_create(socket_t **this, int type, int protocol);
void unix_socket_test(void);

#endif // UNIX_SOCKET