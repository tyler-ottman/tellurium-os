#include "FbContext.hpp"
#include "syscalls.hpp"

FbContext *FbContext::instance;

FbContext::FbContext() {

}

FbContext::~FbContext() {
    
}

FbContext *FbContext::getInstance() {
    if (instance == nullptr) {
        instance = new FbContext();

        if (!syscall_get_fb_context(&instance->fb_meta) ||
            !syscall_get_fb_buffer((void **)&instance->fb_buff)) {
            return nullptr;
        }
    }
    return instance;
}

FbMeta FbContext::getFbMeta() {
    return instance->fb_meta;
}

uint32_t *FbContext::getFbBuffer() {
    return instance->fb_buff;
}