#include "FbContext.hpp"
#include "ulibc/mem.hpp"
#include "ulibc/syscalls.hpp"

namespace GUI {

FbContext *FbContext::instance;

FbContext::FbContext() {

}

FbContext::~FbContext() {
    
}

FbContext *FbContext::getInstance() {
    if (instance == nullptr) {
        instance = new FbContext();
        if (!syscall_get_fb_context(&instance->fb_meta)) {
            return nullptr;
        }
    }
    return instance;
}

FbMeta *FbContext::getFbContext() {
    return &instance->fb_meta;
}

void FbContext::drawRect(int x_pos, int y_pos, int width, int height,
                         uint32_t color) {
    uint32_t x_max = x_pos + width;
    uint32_t y_max = y_pos + height;

    FbMeta &fb_meta = instance->fb_meta;
    uint32_t *buff = (uint32_t *)fb_meta.fb_buff;

    if(x_max > fb_meta.fb_width) {
        x_max = fb_meta.fb_width; 
    }   

    if(y_max > fb_meta.fb_height) {
        y_max = fb_meta.fb_height;
    }

    for(size_t i = y_pos; i < y_max; i++) {
        for (size_t j = x_pos; j < x_max; j++) {
            buff[i * fb_meta.fb_width + j] = color;
        }
    }
}

}