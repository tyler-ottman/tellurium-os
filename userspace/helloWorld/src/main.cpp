#include "FbContext.hpp"
#include <stddef.h>
#include <stdint.h>
#include "syscalls.hpp"

int main() {
    // FbContext *fbContext = FbContext::getInstance();

    // FbMeta fbMeta = fbContext->getFbMeta();
    // uint32_t *fbBuff = fbContext->getFbBuffer();

    FbMeta context;
    uint32_t *fbBuff;
    syscall_get_fb_context(&context);
    syscall_get_fb_buffer((void **)&fbBuff);
    
    for (size_t i = 0; i < 10000; i++) {
        fbBuff[i] = 0xffffffff;
    }

    for (;;) {}
}