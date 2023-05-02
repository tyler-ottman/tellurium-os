#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "FbContext.hpp"

size_t syscall_get_fb_context(FbMeta *fb_meta);
int syscall_get_fb_buffer(void **buff);
void *syscall_mmap(void *addr, size_t len, int prot, int flags, int fd, size_t offset);
#endif // SYSCALLS_H