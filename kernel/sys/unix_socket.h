#ifndef UNIX_SOCKET
#define UNIX_SOCKET

#include <sys/socket.h>

int unix_socket_create(socket_t **this, int type, int protocol);

#endif // UNIX_SOCKET