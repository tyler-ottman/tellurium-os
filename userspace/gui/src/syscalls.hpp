#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "FbContext.hpp"

size_t syscall_get_fb_context(FbMeta *fb_meta);
void *syscall_mmap(void *addr, size_t len, int prot, int flags, int fd, size_t offset);
int syscall_open(const char *path, int flags);
#endif // SYSCALLS_H