#include "Windowing/FbContext.hpp"
#include <stddef.h>
#include <stdint.h>
#include "ulibc/syscalls.hpp"

int main() {
    FbContext *fbContext = FbContext::getInstance();
    uint32_t *fbBuff = (uint32_t *)fbContext->getFbContext()->fb_buff;
    
    FbMeta *meta = fbContext->getFbContext();
    int num_pixels = meta->fb_height * meta->fb_width;

    // for (int i = 0; i < num_pixels; i++) {
    //     fbBuff[i] = i * i / 32;
    // }

    for (;;) {}
}