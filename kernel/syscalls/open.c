#include <arch/cpu.h>
#include <arch/syscalls.h>
#include <arch/process.h>
#include <fs/fd.h>
#include <fs/vfs.h>
#include <memory/vmm.h>

#define O_WRONLY                    0x01
#define O_RDONLY                    0x02
#define O_RDWR                      0x03

int syscall_open(const char *path, int flags) {
    core_t *core = get_core_local_info();
    pcb_t *proc = core->current_thread->parent;

    fd_t *fd;
    int fd_i;
    int err = fd_create(&proc->fd_table, &fd, &fd_i);
    if (err == FD_FAIL) {
        return -1;
    }

    fd->flags = flags;

    vnode_t *vnode = NULL;
    err = vfs_open(&vnode, vfs_get_root(), path);
    if (err) {
        return -1;
    }

    fd->vnode = vnode;

    return fd_i;
}