#include <arch/cpu.h>
#include <arch/process.h>
#include <arch/syscalls.h>
#include <flibc/fcntl.h>
#include <fs/fd.h>
#include <fs/vfs.h>
#include <memory/vmm.h>
#include <sys/misc.h>
#include <sys/stat.h>

int syscall_open(const char *path, int flags) {
    return syscall_openat(AT_FDCWD, path, flags);
}

int syscall_openat(int dirfd, const char *path, int flags) {
    ASSERT_RET(path, -1);

    pcb_t *proc = get_thread_local()->parent;
    fd_table_t *fd_table = &proc->fd_table;

    fd_t *fd;
    int fd_i;
    int err = fd_create(fd_table, &fd, &fd_i);
    if (err == FD_FAIL) {
        return -1;
    }

    fd->flags = flags;

    vnode_t *v_relative = proc->cwd;
    fd_t *fd_relative = NULL;
    bool is_fd_relative = false;
    
    // Path might be relative to dirfd
    if (dirfd != AT_FDCWD) {
        err = fd_acquire(fd_table, &fd_relative, dirfd);
        if (err) {
            goto syscall_openat_fail;
        }

        v_relative = fd_relative->vnode;
        is_fd_relative = true;
    }

    vnode_t *vbase = vfs_get_root();
    if (*path != '/') {
        vbase = is_fd_relative ? v_relative : proc->cwd;
    }
    
    vnode_t *vnode = NULL;
    err = vfs_open(&vnode, vbase, path);
    if (err == VFS_NODE_MISSING && (flags & O_CREAT)) {
        err = vfs_create(vbase, path, VREG);
        if (err) {
            goto syscall_openat_fail;
        }

        fd->flags &= ~O_CREAT;

        err = vfs_open(&vnode, vbase, path);
        if (err) {
            goto syscall_openat_fail;
        }
    } else if (err) { // Open fails for other bad reason
        goto syscall_openat_fail;
    }

    int mode = vnode->stat.st_mode;
    if (S_ISSOCK(mode) ||
        (!S_ISDIR(mode) && (flags & O_DIRECTORY))) {
        goto syscall_openat_fail;
    }

    fd->vnode = vnode;
    fd->mode = vnode->stat.st_mode;
    fd->offset = 0;

    fd_release(fd_table, fd_i);
    if (dirfd != AT_FDCWD) {
        fd_release(fd_table, dirfd);
    }

    return fd_i;

syscall_openat_fail:
    fd_release(fd_table, fd_i);
    fd_free(fd_table, fd_i);
    if (dirfd != AT_FDCWD) {
        fd_release(fd_table, dirfd);
    }
    
    if (vnode) {
        vfs_close(vnode);
    }

    return -1;
}