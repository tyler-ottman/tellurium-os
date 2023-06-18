#pragma once

#include <stdint.h>
#include "Windowing/Rect.hpp"

namespace GUI {

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
    void drawRect(int x_pos, int y_pos, int width, int height,
                  uint32_t color);
};

}