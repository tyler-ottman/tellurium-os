#include <cstddef>
#include "syscalls.hpp"
#include <unistd.h>

#define SYSCALL_GET_FB_CONTEXT                      1
#define SYSCALL_GET_FB_BUFFER                       2
#define SYSCALL_MMAP                                3

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

size_t syscall_get_fb_context(FbMeta *fb_meta) {
    return _syscall(SYSCALL_GET_FB_CONTEXT, fb_meta, 0, 0, 0, 0, 0);
}

int syscall_get_fb_buffer(void **buff) {
    return _syscall(SYSCALL_GET_FB_BUFFER, buff, 0, 0, 0, 0, 0);
}