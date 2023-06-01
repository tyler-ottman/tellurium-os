#include <arch/cpu.h>
#include <arch/process.h>
#include <arch/terminal.h>
#include <fs/fd.h>
#include <libc/kmalloc.h>
#include <libc/ringbuffer.h>
#include <libc/string.h>
#include <stdbool.h>
#include <sockets/unix_socket.h>
#include <sys/misc.h>

#define UNIX_SOCK_BUFF_SIZE                         0x2000

typedef struct unix_socket {
    struct socket socket;
    RINGBUFFER_DECLARE(ringbuffer);
} unix_socket_t;

static int unix_socket_accept(struct socket *this, struct socket **sock,
                              struct sockaddr *addr, socklen_t *addrlen,
                              int fd_flags) {
    ASSERT_RET(this && addr && (*addrlen <= sizeof(struct sockaddr_un)),
               SKT_BAD_PARAM);

    if (this->state != SOCKET_LISTENING) {
        return SKT_BAD_STATE;
    }

    // Create socket that will listen to client
    socket_t *listen_sock;
    unix_socket_create(&listen_sock, this->type, this->protocol);
    if (!listen_sock) {
        return SKT_NO_MEM;
    }

    // Get client socket off backlog
    int err;
    socket_t *client_sock;

    spinlock_acquire(&this->lock);

    while (this->backlog_size == 0) {
        if (fd_flags & O_NONBLOCK) {
            spinlock_release(&this->lock);
            return SKT_NONBLOCK;
        }

        spinlock_release(&this->lock);

        err = event_wait(&this->connection_request);
        if (err == EVENT_ERR) {
            return SKT_BAD_EVENT;
        }

        spinlock_acquire(&this->lock);
    }

    err = socket_pop_from_backlog(this, &client_sock);
    if (err) {
        spinlock_release(&this->lock);
        return err;
    }

    listen_sock->peer = client_sock;
    listen_sock->state = SOCKET_CONNECTED;

    client_sock->peer = listen_sock;
    client_sock->state = SOCKET_CONNECTED;

    // Copy client local addres to sockaddr if non-NULL
    struct sockaddr_un *unix_addr = (struct sockaddr_un *)addr;
    if (!unix_addr) {
        __memcpy(unix_addr, client_sock->local_addr, sizeof(unix_addr));
    }

    // Return new listern socket
    *sock = listen_sock;

    // Signal to client that connection made
    err = event_signal(&client_sock->connection_accepted);
    if (err) {
        spinlock_release(&this->lock);
        return SKT_BAD_EVENT;
    }

    spinlock_release(&this->lock);
    
    return SKT_OK;
}

static int unix_socket_bind(socket_t *this, const struct sockaddr *addr,
                            socklen_t addrlen) {
    ASSERT_RET(this && addr, SKT_BAD_PARAM);

    ASSERT_RET(addr->sa_family == AF_UNIX, SKT_INVALID_DOMAIN);

    // Abstract addresses unsupported
    ASSERT_RET(addr->sa_data != NULL, SKT_BAD_SOCKADDR);

    // Add socket to vfs
    struct sockaddr_un *unix_addr = (struct sockaddr_un *)addr;
    __memcpy(unix_addr->sun_path, unix_addr->sun_path, addrlen);

    pcb_t *proc = get_thread_local()->parent;
    vnode_t *base = proc_get_vnode_base(proc, unix_addr->sun_path);

    int err = vfs_create(base, (const char *)unix_addr->sun_path, VSKT);
    if (err) {
        return SKT_BIND_FAIL;
    }

    // Write socket struct to vfs
    vnode_t *socket_file;
    err = vfs_open(&socket_file, base, unix_addr->sun_path);
    if (err) {
        return SKT_VFS_FAIL;
    }

    socket_file->fs_data = this;

    // Store unix address info in socket
    spinlock_acquire(&this->lock);
    
    __memcpy(this->local_addr, unix_addr, sizeof(struct sockaddr_un));
    this->state = SOCKET_BOUNDED;

    spinlock_release(&this->lock);

    return SKT_OK;
}

static int unix_socket_connect(struct socket *this, const struct sockaddr *addr,
                               socklen_t addrlen) {
    ASSERT_RET(addrlen <= sizeof(struct sockaddr_un), SKT_BAD_PARAM);

    struct sockaddr_un *unix_addr = (struct sockaddr_un *)addr;
    ASSERT_RET(unix_addr->sun_family == AF_UNIX, SKT_INVALID_DOMAIN);

    pcb_t *proc = get_thread_local()->parent;

    int err;
    int state = this->state;

    if (state != SOCKET_CREATED) {
        return SKT_BAD_PARAM;
    }

    // Open peer socket file
    vnode_t *vnode;
    vnode_t *base = proc_get_vnode_base(proc, unix_addr->sun_path);
    err = vfs_open(&vnode, base, unix_addr->sun_path);
    if (err) {
        return SKT_VFS_FAIL;
    }

    if (!S_ISSOCK(vnode->stat.st_mode)) {
        return SKT_BAD_PARAM;
    }

    socket_t *peer = (socket_t *)vnode->fs_data;

    spinlock_acquire(&peer->lock);

    if (peer->state != SOCKET_LISTENING) {
        spinlock_release(&peer->lock);
        return SKT_BAD_PARAM;
    }

    // Add socket to peer's connection list
    err = socket_add_to_peer_backlog(this, peer);
    if (err != SKT_OK) {
        spinlock_release(&peer->lock);
        return err;
    }

    // Signal socket wants to connect to peer
    err = event_signal(&peer->connection_request);
    if (err) {
        socket_backlog_remove(peer, this);
        spinlock_release(&peer->lock);
        return SKT_BAD_EVENT;
    }

    spinlock_release(&peer->lock);

    // Wait until connection accepted
    err = event_wait(&this->connection_accepted);
    if (err == EVENT_ERR) {
        return SKT_BLOCK_FAIL;
    }

    if (vnode) {
        vfs_close(vnode);
    }

    return SKT_OK;
}

static int unix_socket_getpeername(socket_t *this, struct sockaddr *addr,
                                   socklen_t *socklen) {
    ASSERT_RET(this && addr && socklen, SKT_BAD_PARAM);

    spinlock_acquire(&this->lock);

    // Can only reed peer address if connected
    if (this->state != SOCKET_CONNECTED) {
        spinlock_release(&this->lock);
        return SKT_BAD_STATE;
    }

    spinlock_acquire(&this->peer->lock);

    struct sockaddr_un *unix_addr = (struct sockaddr_un *)addr;
    *socklen = MAX(*socklen, sizeof(unix_addr));

    __memcpy(unix_addr, this->peer->local_addr, *socklen);

    spinlock_release(&this->lock);
    spinlock_release(&this->peer->lock);

    return SKT_OK;
}

static int unix_socket_getsockname(socket_t *this, struct sockaddr *addr,
                                   socklen_t *socklen) {
    ASSERT_RET(this && addr && socklen, SKT_BAD_PARAM);

    spinlock_acquire(&this->lock);

    // Can only read sock address if bound
    if (this->state != SOCKET_BOUNDED) {
        spinlock_release(&this->lock);
        return SKT_BAD_STATE;
    }

    struct sockaddr_un *unix_addr = (struct sockaddr_un *)addr;
    *socklen = MAX(*socklen, sizeof(unix_addr));

    __memcpy(unix_addr, this->local_addr, *socklen);

    spinlock_release(&this->lock);

    return SKT_OK;
}

static int unix_socket_listen(socket_t *this, int backlog) {
    ASSERT_RET(this, SKT_BAD_PARAM);
    ASSERT_RET(backlog >= 0, SKT_BACKLOG_CAPACITY_INVALID);
    ASSERT_RET(this->state == SOCKET_BOUNDED, SKT_BAD_STATE);

    spinlock_acquire(&this->lock);

    this->backlog_capacity = MIN(backlog, SKT_BACKLOG_CAPACITY);
    this->backlog_size = 0;
    this->backlog = kmalloc(this->backlog_capacity * sizeof(socket_t *));
    
    if (!this->backlog) {
        spinlock_release(&this->lock);
        return SKT_NO_MEM;
    }

    for (size_t i = 0; i < this->backlog_capacity; i++) {
        this->backlog[i] = NULL;
    }

    this->state = SOCKET_LISTENING;

    spinlock_release(&this->lock);

    return SKT_OK;
}

static size_t unix_socket_recv(struct socket *this, void *buff, size_t len,
                               int flags, int fd_flags, int *bytes_read) {
    ASSERT_RET(this && buff && bytes_read, SKT_BAD_PARAM);

    // No flags functionality
    ASSERT_RET(!flags, SKT_BAD_OP);

    spinlock_acquire(&this->lock);
    
    int err;
    if (this->state != SOCKET_CONNECTED) {
        err = SKT_BAD_STATE;
        goto unix_socket_recv_cleanup;
    }

    unix_socket_t *unix_this = (unix_socket_t *)this;

    // Wait for bytes while nothing on buffer
    while (RINGBUFFER_SIZE(unix_this->ringbuffer) == 0) {
        if (fd_flags & O_NONBLOCK) {
            err = SKT_NONBLOCK;
            goto unix_socket_recv_cleanup;
        }

        spinlock_release(&this->lock);

        err = event_wait(&unix_this->socket.data_received);

        if (err == EVENT_ERR) {
            err = SKT_BLOCK_FAIL;
            goto unix_socket_recv_cleanup;
        }

        spinlock_acquire(&this->lock);
    }

    *bytes_read = RINGBUFFER_READ(unix_this->ringbuffer, buff, len);

    // If peer blocked when trying to write
    event_signal(&this->peer->data_received);

    err = SKT_OK;

unix_socket_recv_cleanup:
    spinlock_release(&this->lock);

    return err;
}

static size_t unix_socket_send(struct socket *this, const void *buff,
                               size_t len, int flags, int fd_flags,
                               int *bytes_written) {
    ASSERT_RET(this && buff, SKT_BAD_PARAM);

    // No flags functionality
    ASSERT_RET(!flags, SKT_BAD_OP);

    spinlock_acquire(&this->peer->lock);

    unix_socket_t *unix_peer = (unix_socket_t *)this->peer;

    int err;
    if (this->peer->state != SOCKET_CONNECTED) {
        err = SKT_BAD_STATE;
        goto unix_socket_send_cleanup;
    }

    while (RINGBUFFER_SIZE(unix_peer->ringbuffer) ==
           RINGBUFFER_CAPACITY(unix_peer->ringbuffer)) {
        if (fd_flags & O_NONBLOCK) {
            err = SKT_NONBLOCK;
            goto unix_socket_send_cleanup;
        }

        spinlock_release(&this->peer->lock);

        err = event_wait(&unix_peer->socket.data_received);
        if (err == EVENT_ERR) {
            err = SKT_BLOCK_FAIL;
            goto unix_socket_send_cleanup;
        }

        spinlock_acquire(&this->peer->lock);
    }

    *bytes_written = RINGBUFFER_WRITE(unix_peer->ringbuffer, buff, len);

    // If peer blocked when trying to read
    event_signal(&this->peer->data_received);

    err = SKT_OK;

unix_socket_send_cleanup:
    spinlock_release(&this->peer->lock);

    return err;
}

int unix_socket_create(socket_t **this, int type, int protocol) {
    ASSERT_RET(type == SOCK_STREAM, SKT_BAD_PARAM);

    unix_socket_t *unix_socket = kmalloc(sizeof(unix_socket_t));
    ASSERT_RET(unix_socket, SKT_NO_MEM);

    int err = socket_init((socket_t *)unix_socket, AF_UNIX, type, protocol);
    ASSERT_RET(err == SKT_OK, err);

    err = RINGBUFFER_ALLOC(unix_socket->ringbuffer, UNIX_SOCK_BUFF_SIZE);
    ASSERT_RET(err == RINGBUFFER_OK, SKT_NO_MEM);
    
    unix_socket->socket.socket_accept = unix_socket_accept;
    unix_socket->socket.socket_bind = unix_socket_bind;
    unix_socket->socket.socket_connect = unix_socket_connect;
    unix_socket->socket.socket_getpeername = unix_socket_getpeername;
    unix_socket->socket.socket_getsockname = unix_socket_getsockname;
    unix_socket->socket.socket_listen = unix_socket_listen;
    unix_socket->socket.socket_recv = unix_socket_recv;
    unix_socket->socket.socket_send = unix_socket_send;

    *this = (socket_t *)unix_socket;

    return SKT_OK;
}