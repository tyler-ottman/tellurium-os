#ifndef SYSCALL_H
#define SYSCALL_H

#define SYS_ERROR                                   0
#define SYS_SUCCESS                                 1

#include <modules/framebuffer.h>

int syscall_get_fb_context(fb_context_t *context);
void *syscall_mmap(void *addr, size_t len, int prot, int flags, int fd, size_t offset);
int syscall_open(const char *path, int flags);
int syscall_openat(int fd, const char *path, int flags);
size_t syscall_read(int fd, void *buf, size_t count);
int syscall_lseek(int fd, size_t offset, int whence);

void init_syscall(void);

#endif // SYSCALL_H