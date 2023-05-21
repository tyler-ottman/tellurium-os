#include <arch/cpu.h>
#include <arch/process.h>
#include <arch/terminal.h>
#include <fs/fd.h>
#include <libc/kmalloc.h>
#include <libc/ringbuffer.h>
#include <libc/string.h>
#include <stdbool.h>
#include <sys/misc.h>
#include <sys/unix_socket.h>

#define UNIX_SOCK_BUFF_SIZE                         0x2000

typedef struct unix_socket {
    socket_t socket;
    RINGBUFFER_DECLARE(ringbuffer);
} unix_socket_t;

static int unix_socket_accept(struct socket *this, struct socket **sock,
                              struct sockaddr *addr, socklen_t *addrlen,
                              int fd_flags) {
    ASSERT_RET(this && addr && (*addrlen <= sizeof(struct sockaddr_un)),
               SKT_BAD_PARAM);

    spinlock_acquire(&this->lock);

    if (this->state != SOCKET_LISTENING) {
        spinlock_release(&this->lock);
        return SKT_BAD_STATE;
    }

    // Create socket that will listen to client
    socket_t *listen_sock;
    unix_socket_create(&listen_sock, this->type, this->protocol);
    if (!listen_sock) {
        return SKT_NO_MEM;
    }

    // Get client socket off backlog
    socket_t *client_sock;
    int err = socket_pop_from_backlog(this, &client_sock);
    if (err != SKT_OK) {
        spinlock_release(&this->lock);
        return err;
    }

    if (!client_sock) { // Backlog empty, wait for connections
        if (fd_flags & O_NONBLOCK) {
            spinlock_release(&this->lock);
            return SKT_NONBLOCK;
        }

        err = event_wait(&this->connection_request);
        if (err == EVENT_ERR) {
            spinlock_release(&this->lock);
            return SKT_BAD_EVENT;
        }

        socket_pop_from_backlog(this, &client_sock);
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
    event_signal(&client_sock->connection_accepted);

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

    pcb_t *proc = get_core_local_info()->current_thread->parent;
    vnode_t *base = proc_get_vnode_base(proc, unix_addr->sun_path);
    int err = vfs_create(base, (const char *)unix_addr->sun_path, VSKT);
    if (!err) {
        return SKT_BIND_FAIL;
    }

    // Write socket struct to vfs
    vnode_t *socket_file;
    err = vfs_open(&socket_file, base, unix_addr->sun_path);
    if (!err) {
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

    struct core_local_info *cpu_info = get_core_local_info();
    pcb_t *proc = cpu_info->current_thread->parent;

    spinlock_acquire(&this->lock);

    int err;

    int state = this->state;
    if (state == SOCKET_LISTENING || state == SOCKET_CONNECTED) {
        err = SKT_BAD_PARAM;
        goto unix_socket_connect_cleanup;
    }

    // Open peer socket file
    vnode_t *vnode;
    vnode_t *base = proc_get_vnode_base(proc, unix_addr->sun_path);
    err = vfs_open(&vnode, base, unix_addr->sun_path);
    if (!err) {
        err = SKT_VFS_FAIL;
        goto unix_socket_connect_cleanup;
    }

    if (!S_ISSOCK(vnode->stat.st_mode)) {
        err = SKT_BAD_PARAM;
        goto unix_socket_connect_cleanup;
    }

    socket_t *peer = (socket_t *)vnode->fs_data;

    // Add socket to peer's connection list
    err = socket_add_to_peer_backlog(this, peer);
    if (err != SKT_OK) {
        goto unix_socket_connect_cleanup;
    }

    // Signal socket wants to connect to peer
    event_signal(&peer->connection_request);

    // Wait until connection accepted
    err = event_wait(&this->connection_accepted);
    if (err == EVENT_ERR) {
        err = SKT_BLOCK_FAIL;
        goto unix_socket_connect_cleanup;
    }

    if (vnode) {
        vfs_close(vnode);
    }

    err = SKT_OK;
unix_socket_connect_cleanup:
    spinlock_release(&this->lock);
    return err;
}

static int unix_socket_getpeername(socket_t *this, struct sockaddr *addr,
                                   socklen_t *socklen) {
    ASSERT_RET(this && addr && socklen, SKT_BAD_PARAM);

    spinlock_acquire(&this->lock);

    // Can only reed peer address if connected
    int err;
    if (this->state != SOCKET_CONNECTED) {
        err = SKT_BAD_STATE;
        goto unix_socket_getpeername_cleanup;
    }

    spinlock_acquire(&this->peer->lock);

    struct sockaddr_un *unix_addr = (struct sockaddr_un *)addr;
    *socklen = MAX(*socklen, sizeof(unix_addr));

    __memcpy(unix_addr, this->peer->local_addr, *socklen);
    err = SKT_OK;

unix_socket_getpeername_cleanup:
    spinlock_release(&this->lock);
    spinlock_release(&this->peer->lock);

    return err;
}

static int unix_socket_getsockname(socket_t *this, struct sockaddr *addr,
                                   socklen_t *socklen) {
    ASSERT_RET(this && addr && socklen, SKT_BAD_PARAM);

    spinlock_acquire(&this->lock);

    // Can only read sock address if bound
    int err;
    if (this->state != SOCKET_BOUNDED) {
        err = SKT_BAD_STATE;
        goto unix_socket_getpeername_cleanup;
    }

    struct sockaddr_un *unix_addr = (struct sockaddr_un *)addr;
    *socklen = MAX(*socklen, sizeof(unix_addr));

    __memcpy(unix_addr, this->local_addr, *socklen);
    err = SKT_OK;

unix_socket_getpeername_cleanup:
    spinlock_release(&this->lock);

    return err;
}

static int unix_socket_listen(socket_t *this, int backlog) {
    ASSERT_RET(this, SKT_BAD_PARAM);
    ASSERT_RET(backlog >= 0, SKT_BACKLOG_CAPACITY_INVALID);
    ASSERT_RET(this->state == SOCKET_BOUNDED, SKT_BAD_STATE);

    spinlock_acquire(&this->lock);

    this->backlog_capacity = MIN(backlog, SKT_BACKLOG_CAPACITY);
    this->backlog_size = 0;
    this->backlog = kmalloc(this->backlog_capacity * sizeof(socket_t *));
    
    int err;
    if (!this->backlog) {
        err = SKT_NO_MEM;
        goto socket_listen_cleanup;
    }

    for (size_t i = 0; i < this->backlog_capacity; i++) {
        this->backlog[i] = NULL;
    }

    this->state = SOCKET_LISTENING;

    err = SKT_OK;
socket_listen_cleanup:
    spinlock_release(&this->lock);
    return err;
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