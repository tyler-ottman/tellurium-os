#ifndef SOCKET_H
#define SOCKET_H

#include <stdint.h>
#include <stddef.h>

#define AF_UNIX_PATH_LEN                        108

#define SOCKETADDR_STORAGE_SIZE                 128

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

enum socket_state {
    SOCKET_CREATED,
    SOCKET_BOUND
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
    int domain;
    int type;
    int protocol;
    int state;

    struct socketaddr_storage *local_addr;
    
    struct socket *peer;

    int (*socket_bind)(struct socket *this, const struct sockaddr *addr, socklen_t addrlen);
} socket_t;

int socket_init(socket_t *this, int domain, int type, int protocol);

#endif // SOCKET_H