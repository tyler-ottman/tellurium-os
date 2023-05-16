#ifndef FD_H
#define FD_H

#include <arch/cpu.h>
#include <arch/lock.h>
#include <fs/vfs.h>
#include <stddef.h>
#include <stdint.h>

#define FD_SUCCESS                  1
#define FD_FAIL                     0

// Fd flags
#define O_NONBLOCK                  00004000

typedef struct fd {
    spinlock_t fd_lock;
    vnode_t *vnode;
    size_t ref;
    size_t offset;
    int flags;
    int mode;
} fd_t;

typedef struct fd_table {
    spinlock_t fd_table_lock;
    VECTOR_DECLARE(fd_table);
} fd_table_t;

int fd_create(fd_table_t *fd_table, fd_t **fd, int *fd_i);
int fd_free(fd_table_t *fd_table, int fd_i);

int fd_access(fd_table_t *fd_table, fd_t **fd, int fd_i);
int fd_release(fd_table_t *fd_table, int fd_i);

#endif // FD_H