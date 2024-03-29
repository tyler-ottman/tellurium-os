#ifndef SOCKET_H
#define SOCKET_H

#include <arch/lock.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/event.h>

#define AF_UNIX_PATH_LEN                        108

#define SOCKETADDR_STORAGE_SIZE                 128
#define SKT_BACKLOG_CAPACITY                    100

// Socket Family
#define AF_UNIX                                 1
#define AF_INET                                 2
#define AF_INIET6                               10

// Socket Types
#define SOCK_STREAM                             1
#define SOCK_DGRAM                              2

// Socket Return codes
#define SKT_OK                                  0
#define SKT_BAD_PARAM                           1
#define SKT_INVALID_DOMAIN                      2
#define SKT_NO_MEM                              3
#define SKT_BAD_OP                              4
#define SKT_BAD_SOCKADDR                        5
#define SKT_BIND_FAIL                           6
#define SKT_BAD_STATE                           7
#define SKT_VFS_FAIL                            8
#define SKT_BACKLOG_FULL                        9
#define SKT_BLOCK_FAIL                          10
#define SKT_BACKLOG_EMPTY                       11
#define SKT_BACKLOG_CAPACITY_INVALID            12
#define SKT_BAD_EVENT                           13
#define SKT_NONBLOCK                            14

enum socket_state {
    SOCKET_CREATED,
    SOCKET_BOUNDED,
    SOCKET_LISTENING,
    SOCKET_CONNECTED
};

typedef unsigned socklen_t;
typedef unsigned sa_family_t;

struct iovec {
   uintptr_t iov_base;
   size_t iov_len;
};

// Generic sockaddr
struct sockaddr {
    sa_family_t sa_family;              // Address family
    char sa_data[14];                   // Socket Address
};

// Unix sockaddr format
struct sockaddr_un {
    sa_family_t sun_family;
    char sun_path[AF_UNIX_PATH_LEN];
};

struct socketaddr_storage {
    sa_family_t ss_family;
    char pad[SOCKETADDR_STORAGE_SIZE - sizeof(sa_family_t)];
};

struct msghdr {
    void *msg_name;
    socklen_t msg_namelen;
    struct iovec *msg_iov;
    int msg_iovlen;
    void *msg_control;
    socklen_t msg_controllen;
    int msg_flags;
};

typedef struct socket {
    spinlock_t lock;
    int domain;
    int type;
    int protocol;
    int state;
    struct socketaddr_storage *local_addr;
    struct socket *peer;
    
    event_t connection_request;
    event_t connection_accepted;
    event_t data_received;

    struct socket **backlog;
    size_t backlog_capacity;
    size_t backlog_size;

    int (*socket_accept)(struct socket *this, struct socket **sock,
                         struct sockaddr *addr, socklen_t *addrlen,
                         int fd_flags);
    int (*socket_bind)(struct socket *this, const struct sockaddr *addr,
                       socklen_t addrlen);
    int (*socket_connect)(struct socket *this, const struct sockaddr *addr,
                          socklen_t addrlen);
    int (*socket_getpeername)(struct socket *this, struct sockaddr *addr,
                              socklen_t *socklen);
    int (*socket_getsockname)(struct socket *this, struct sockaddr *addr,
                              socklen_t *socklen);
    int (*socket_listen)(struct socket *this, int backlog);
    size_t (*socket_recv)(struct socket *this, void *buff, size_t len,
                          int flags, int fd_flags, int *bytes_read);
    size_t (*socket_send)(struct socket *this, const void *buff, size_t len,
                          int flags, int fd_flags, int *bytes_written);
} socket_t;

int socket_init(socket_t *this, int domain, int type, int protocol);
int socket_add_to_peer_backlog(socket_t *this, socket_t *peer);
int socket_backlog_remove(socket_t *this, socket_t *remove);
int socket_pop_from_backlog(socket_t *this, socket_t **pop);

#endif // SOCKET_H