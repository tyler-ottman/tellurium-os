#include <libc/kmalloc.h>
#include <sys/socket.h>
#include <sys/unix_socket.h>

int socket_init(socket_t *this, int domain, int type, int protocol) {
    if (!this) {
        return SKT_BAD_PARAM;
    }

    this->domain = domain;
    this->type = type;
    this->protocol = protocol;
    this->peer = NULL;
    this->state = SOCKET_CREATED;

    this->local_addr = kmalloc(sizeof(struct socketaddr_storage));
    if (!this->local_addr) {
        return SKT_NO_MEM;
    }
    this->peer = NULL;

    this->socket_bind = NULL;

    return SKT_OK;
}