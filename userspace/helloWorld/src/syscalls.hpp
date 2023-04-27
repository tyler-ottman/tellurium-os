#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "FbContext.hpp"

int syscall_get_fb_context(FbMeta *fb_meta);
int syscall_get_fb_buffer(void **buff);

#endif // SYSCALLS_H