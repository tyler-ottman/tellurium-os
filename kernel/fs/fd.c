#include <fs/fd.h>
#include <klib/kmalloc.h>

int fd_create(fd_table_t *fd_table, fd_t **fd, int *fd_i) {
    spinlock_acquire(&fd_table->fd_table_lock);

    // Find empty file descriptor entry in table
    int fd_idx = -1;
    for (int i = 0; i < VECTOR_SIZE(fd_table->fd_table); i++) {
        if (!VECTOR_GET(fd_table->fd_table, i)) {
            fd_idx = i;
            break;
        }
    }

    fd_t *new_fd = kmalloc(sizeof(fd_t));
    if (!new_fd) {
        spinlock_release(&fd_table->fd_table_lock);
        return FD_FAIL;
    }

    new_fd->ref = 1;
    new_fd->fd_lock = 0;
    new_fd->vnode = NULL;

    if (fd_idx == -1) { // Resize fd table
        size_t old_size = VECTOR_SIZE(fd_table->fd_table);

        VECTOR_PUSH_BACK(fd_table->fd_table, new_fd);
        for (int i = old_size; i < VECTOR_SIZE(fd_table->fd_table); i++) {
            if (VECTOR_GET(fd_table->fd_table, i) == new_fd) {
                fd_idx = i;
                break;
            }
        }

        if (fd_idx == -1) {
            spinlock_release(&fd_table->fd_table_lock);
            return FD_FAIL;
        }
    }

    *fd = new_fd;
    *fd_i = fd_idx;

    // spinlock_acquire(&new_fd->fd_lock);
    spinlock_release(&fd_table->fd_table_lock);

    return FD_SUCCESS;
}

int fd_free(fd_table_t *fd_table, int fd_i) {
    spinlock_acquire(&fd_table->fd_table_lock);

    vector_t fd_table_vec = fd_table->fd_table;
    fd_t *fd = VECTOR_GET(fd_table_vec, fd_i);
    if (!fd) {
        spinlock_release(&fd_table->fd_table_lock);
        return FD_FAIL;
    }

    if (fd->ref == 0) {
        VECTOR_SET(fd_table_vec, fd_i, NULL);
        if (fd->vnode) {
            vfs_close(fd->vnode);
        }
        kfree(fd);
    }
    
    spinlock_release(&fd_table->fd_table_lock);

    return FD_SUCCESS;
}

int fd_acquire(fd_table_t *fd_table, fd_t **fd, int fd_i) {    
    spinlock_acquire(&fd_table->fd_table_lock);

    fd_t *fd_ret = VECTOR_GET(fd_table->fd_table, fd_i);
    if (!fd_ret) {
        spinlock_release(&fd_table->fd_table_lock);
        return FD_FAIL;
    }

    *fd = fd_ret;
    fd_t *tmp = *fd;
    spinlock_acquire(&tmp->fd_lock);

    spinlock_release(&fd_table->fd_table_lock);

    return FD_SUCCESS;
}

int fd_release(fd_table_t *fd_table, int fd_i) {
    spinlock_acquire(&fd_table->fd_table_lock);
    
    fd_t *fd = VECTOR_GET(fd_table->fd_table, fd_i);
    if (!fd) {
        return FD_FAIL;
    }

    spinlock_release(&fd_table->fd_table_lock);
    spinlock_release(&fd->fd_lock);

    return FD_SUCCESS;
}