#include <libc/kmalloc.h>
#include <sys/socket.h>
#include <sys/unix_socket.h>

int socket_init(socket_t *this, int domain, int type, int protocol) {
    if (!this) {
        return SKT_BAD_PARAM;
    }

    this->lock = 0;

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

    this->backlog_capacity = SOCKET_BACKLOG_CAPACITY;
    this->backlog_size = 0;
    this->backlog = kmalloc(this->backlog_capacity * sizeof(socket_t *));
    if (!this->backlog) {
        return SKT_NO_MEM;
    }

    this->socket_bind = NULL;

    return SKT_OK;
}

int socket_add_to_peer_backlog(socket_t *this, socket_t *peer) {
    if (!this || !peer) {
        return SKT_BAD_PARAM;
    }

    if (peer->backlog_size == peer->backlog_capacity) {
        return SKT_BACKLOG_FULL;
    }

    peer->backlog[peer->backlog_size++] = this;

    return SKT_OK;
}