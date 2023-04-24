#include <stddef.h>

int proto_syscall(int num) {
    int ret;

    asm volatile(
        "mov %1, %%rax\n\t"
        "syscall\n\t" :
        "=a"(ret) :
        "m"(num)
    );
    
    return ret;
}

int _start() {
    proto_syscall(40);

    for (;;) {}
}