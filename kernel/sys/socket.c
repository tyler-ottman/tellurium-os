#include <libc/kmalloc.h>
#include <sys/misc.h>
#include <sys/socket.h>
#include <sys/misc.h>
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

    this->backlog_capacity = 0;
    this->backlog_size = 0;
    this->backlog = NULL;

    return SKT_OK;
}

int socket_backlog_pop_latest(socket_t *peer) {
    ASSERT_RET(peer, SKT_BAD_PARAM);

    ASSERT_RET(peer->backlog_size != 0, SKT_BACKLOG_EMPTY);

    peer->backlog[peer->backlog_size--] = NULL;

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

int socket_pop_from_backlog(socket_t *this, socket_t **pop) {
    ASSERT_RET(this && pop, SKT_BAD_PARAM);

    if (this->backlog_size == 0) {
        *pop = NULL;
        return SKT_BACKLOG_EMPTY;
    }

    *pop = this->backlog[0];

    for (size_t i = 0; i < this->backlog_size - 1; i++) {
        this->backlog[i] = this->backlog[i + 1];
    }

    return SKT_OK;
}