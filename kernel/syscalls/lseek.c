#include <arch/cpu.h>
#include <arch/process.h>
#include <arch/syscalls.h>
#include <fs/fd.h>
#include <sys/misc.h>
#include <sys/stat.h>

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define LSEEK_ERR                               -1

int syscall_lseek(int fd, size_t offset, int whence) {
    pcb_t *proc = get_thread_local()->parent;
    fd_table_t *fd_table = &proc->fd_table;

    fd_t *file;

    int err = fd_acquire(fd_table, &file, fd);
    if (err) {
        return LSEEK_ERR;
    }
    
    int mode = file->vnode->stat.st_mode;
    if (S_ISSOCK(mode) || S_ISFIFO(mode)) {
        return LSEEK_ERR;
    }

    switch (whence) {
    case SEEK_SET:
        file->offset = offset;
        break;
    case SEEK_CUR:
        file->offset += offset;
        break;
    case SEEK_END:
        file->offset = file->vnode->stat.st_size;
        break;
    }

    fd_release(fd_table, fd);

    return file->offset;
}