#include "FbContext.hpp"
#include <stddef.h>
#include <stdint.h>
#include "syscalls.hpp"

int main() {
    FbContext *fbContext = FbContext::getInstance();

    FbMeta fbMeta = fbContext->getFbMeta();
    uint32_t *fbBuff = fbContext->getFbBuffer();
    
    for (size_t i = 0; i < 10000; i++) {
        fbBuff[i] = 0xffff00ff;
    }

    for (;;) {}
}