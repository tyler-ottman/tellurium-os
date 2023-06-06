#include "FbContext.hpp"
#include <stddef.h>
#include <stdint.h>
#include "syscalls.hpp"

int main() {
    FbContext *fbContext = FbContext::getInstance();
    uint32_t *fbBuff = (uint32_t *)fbContext->getFbContext().fb_buff;

    for (;;) {}
}