#include <arch/cpu.h>
#include <arch/process.h>
#include <libc/kmalloc.h>
#include <libc/string.h>
#include <stdbool.h>
#include <sys/unix_socket.h>

#define UNIX_SOCK_BUFF_SIZE                         0x2000

typedef struct unix_socket {
    socket_t socket;
    void *buff;
    size_t buff_size;
} unix_socket_t;

int unix_socket_bind(socket_t *this, const struct sockaddr *addr, socklen_t addrlen) {
    if (!this || !addr) {
        return SKT_BAD_PARAM;
    }

    if (addr->sa_family != AF_UNIX) {
        return SKT_INVALID_DOMAIN;
    }

    if (!addr->sa_data) { // Abstract addresses unsupported
        return SKT_BAD_SOCKADDR;
    }

    // Add socket to vfs
    struct sockaddr_un *unix_addr = (struct sockaddr_un *)addr;
    struct core_local_info *cpu_info = get_core_local_info();
    pcb_t *proc = cpu_info->current_thread->parent;
    __memcpy(unix_addr->sun_path, unix_addr->sun_path, addrlen);
    int err = vfs_create(proc->cwd, (const char *)unix_addr->sun_path, VREG);
    if (!err) {
        return SKT_BIND_FAIL;
    }

    // Store unix address info in socket
    spinlock_acquire(&this->lock);
    __memcpy(this->local_addr, unix_addr, sizeof(struct sockaddr_un));
    this->state = SOCKET_BOUNDED;
    spinlock_release(&this->lock);

    return SKT_OK;
}

int unix_socket_connect(struct socket *this, const struct sockaddr *addr, socklen_t addrlen) {
    if (addrlen > sizeof(struct sockaddr_un)) {
        return SKT_BAD_PARAM;
    }

    struct sockaddr_un *unix_addr = (struct sockaddr_un *)addr;
    if (unix_addr->sun_family != AF_UNIX) {
        return SKT_INVALID_DOMAIN;
    }

    struct core_local_info *cpu_info = get_core_local_info();
    pcb_t *proc = cpu_info->current_thread->parent;

    spinlock_acquire(&this->lock);

    int err;

    int state = this->state;
    if (state == SOCKET_LISTENING || state == SOCKET_CONNECTED) {
        err = SKT_BAD_PARAM;
        goto unix_socket_connect_fail;
    }

    // Open peer socket file
    vnode_t *vnode;
    err = vfs_open(&vnode, proc->cwd, unix_addr->sun_path);
    if (!err) {
        goto unix_socket_connect_fail;
    }

    socket_t *peer = vnode->fs_data;

    if (!S_ISSOCK(vnode->stat.st_mode)) {
        err = SKT_BAD_PARAM;
        goto unix_socket_connect_fail;
    }

    // Add socket to peer's connection list
    err = socket_add_to_peer_backlog(this, peer);
    if (err != SKT_OK) {
        return err;
    }

    // Signal socket wants to connect to peer
    event_signal(&this->peer->connection_request);

    // Wait until connection acknowledged
    err = event_wait(&this->connection_accepted);
    if (err == EVENT_ERR) {
        err = SKT_BLOCK_FAIL;
        goto unix_socket_connect_fail;
    }

    if (vnode) {
        vfs_close(vnode);
    }

unix_socket_connect_fail:
    spinlock_release(&this->lock);
    return err;
}

int unix_socket_create(socket_t **this, int type, int protocol) {
    if (type != SOCK_STREAM) {
        return SKT_BAD_PARAM;
    }

    unix_socket_t *unix_socket = kmalloc(sizeof(unix_socket_t));
    if (!unix_socket) {
        return SKT_NO_MEM;
    }

    int err = socket_init((socket_t *)unix_socket, AF_UNIX, type, protocol);
    if (err != SKT_OK) {
        return err;
    }

    unix_socket->buff = kmalloc(UNIX_SOCK_BUFF_SIZE);
    if (!unix_socket->buff) {
        return SKT_NO_MEM;
    }
    unix_socket->buff_size = 0;
    
    unix_socket->socket.socket_bind = unix_socket_bind;

    *this = (socket_t *)unix_socket;

    return SKT_OK;
}