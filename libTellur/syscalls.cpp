#include <stddef.h>
#include "syscalls.hpp"
#include "libGUI/FbContext.hpp"

#define SYSCALL_GET_FB_CONTEXT                      1
#define SYSCALL_MMAP                                2
#define SYSCALL_OPEN                                3
#define SYSCALL_READ                                4
#define SYSCALL_LSEEK                               5

template <class T1, class T2, class T3, class T4, class T5, class T6>
size_t _syscall(size_t sys_id,  T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6) {
    size_t ret;
    register auto r10 __asm__ ("r10") = a4;
    register auto r8  __asm__ ("r8")  = a5;
    register auto r9  __asm__ ("r9")  = a6;

    __asm__ volatile(
        "syscall"
        : "=a" (ret)
        : "a" (sys_id), "D"(a1), "S"(a2), "d"(a3), "r"(r10), "r"(r8), "r"(r9)
        : "rcx", "r11", "cc", "memory"
    );

    return ret;
}

size_t syscall_get_fb_context(void *fb_meta) {
    return _syscall(SYSCALL_GET_FB_CONTEXT, fb_meta, 0, 0, 0, 0, 0);
}

void *syscall_mmap(void *addr, size_t len, int prot, int flags, int fd, size_t offset) {
    return (void *)_syscall(SYSCALL_MMAP, addr, len, prot, flags, fd, offset);
}

int syscall_open(const char *path, int flags) {
    return (int)_syscall(SYSCALL_OPEN, path, flags, 0, 0, 0, 0);
}

int syscall_read(int fd, void *buf, size_t count) {
    return (int)_syscall(SYSCALL_READ, fd, buf, count, 0, 0, 0);
}

int syscall_lseek(int fd, size_t offset, int whence) {
    return _syscall(SYSCALL_LSEEK, fd, offset, whence, 0, 0, 0);
}