#include <cstddef>
#include "syscalls.hpp"

int syscall_get_fb_context(FbMeta *fb_meta) {
    size_t ret;
    asm volatile
    (
        "syscall"
        : "=a" (ret)
        : "0"(1), "D"(fb_meta)
        : "rcx", "r11", "memory"
    );
    
    return ret;
}

int syscall_get_fb_buffer(void **buff) {
    size_t ret;
    asm volatile
    (
        "syscall"
        : "=a" (ret)
        : "0"(2), "D"(buff)
        : "rcx", "r11", "memory"
    );
    
    return ret;
}