#ifndef FBCONTEXT_H
#define FBCONTEXT_H

#include <cstdint>

typedef struct {
    void *fb_buff;
    uint32_t fb_width;
    uint32_t fb_height;
    uint32_t fb_pitch;
} FbMeta;

class FbContext {
private:
    FbMeta fb_meta;
    uint32_t *fb_buff;

    static FbContext *instance;
    FbContext();
    ~FbContext();
public:
    FbContext(FbContext& obj) = delete;
    static FbContext *getInstance(void);

    FbMeta *getFbContext(void);
};

#endif // FBCONTEXT_H