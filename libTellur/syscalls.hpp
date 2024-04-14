#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <stdint.h>

struct FbInfo {
    void *fb_buff;
    uint32_t fb_width;
    uint32_t fb_height;
    uint32_t fb_pitch;
    uint32_t fb_bpp;
    FbInfo() : fb_buff(nullptr), fb_width(0), fb_height(0), fb_pitch(0),
        fb_bpp(0) {}
};

size_t syscall_get_fb_context(FbInfo *fb_meta);
void *syscall_mmap(void *addr, size_t len, int prot, int flags, int fd, size_t offset);
int syscall_open(const char *path, int flags);
int syscall_read(int fd, void *buf, size_t count);
int syscall_lseek(int fd, size_t offset, int whence);

#endif // SYSCALLS_H