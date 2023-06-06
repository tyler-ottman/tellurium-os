#include "FbContext.hpp"
#include "mem.hpp"
#include "syscalls.hpp"

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