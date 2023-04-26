#include <stddef.h>
#include <stdint.h>

typedef struct fb_context {
    uint32_t fb_width;
    uint32_t fb_height;
    uint32_t fb_pitch;
} fb_context_t;

int syscall_get_fb_context(fb_context_t *context) {
    size_t ret;
    asm volatile
    (
        "syscall"
        : "=a" (ret)
        : "0"(1), "D"(context)
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

extern "C" int _start() {
    fb_context_t context;
    uint32_t *fb_buff;
    syscall_get_fb_context(&context);

    uint32_t *buff;
    syscall_get_fb_buffer((void **)&buff);
    
    for (size_t i = 0; i < 10000; i++) {
        buff[i] = 0xffffffff;
    }

    for (;;) {}
}