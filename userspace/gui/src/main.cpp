#include "FbContext.hpp"
#include <stddef.h>
#include <stdint.h>
#include "syscalls.hpp"

int main() {
    FbContext *fbContext = FbContext::getInstance();
    uint32_t *fbBuff = (uint32_t *)fbContext->getFbContext().fb_buff;
    
    int fd = syscall_open("/var/temp/test", 3);

    // for (size_t i = 0; i < 100000; i++) {
    //     fbBuff[i] = 0xffff00ff;
    // }

    for (;;) {}
}