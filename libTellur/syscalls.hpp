#ifndef SYSCALLS_H
#define SYSCALLS_H

size_t syscall_get_fb_context(void *fb_meta);
void *syscall_mmap(void *addr, size_t len, int prot, int flags, int fd, size_t offset);
int syscall_open(const char *path, int flags);
int syscall_read(int fd, void *buf, size_t count);
int syscall_lseek(int fd, size_t offset, int whence);

#endif // SYSCALLS_H