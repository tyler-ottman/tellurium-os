#include <arch/cpu.h>
#include <arch/process.h>
#include <arch/syscalls.h>
#include <sys/misc.h>

size_t syscall_read(int fd, void *buf, size_t count) {
    ASSERT_RET(buf, -1);

    pcb_t *proc = get_thread_local()->parent;        
    fd_t *read_fd;

    int err = fd_acquire(&proc->fd_table, &read_fd, fd);
    if (err) {
        return -1;
    }

    size_t bytes_read;
    err = vfs_read(buf, read_fd->vnode, count, read_fd->offset, &bytes_read);
    if (err) {
        fd_release(&proc->fd_table, fd);
        return -1;
    }

    fd_release(&proc->fd_table, fd);

    if (bytes_read) {
        // kprintf("%c", *((char *)buf));
    }

    return bytes_read;
}