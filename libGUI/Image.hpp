#pragma once

#include "Window.hpp"

#define IMG_OK                  0
#define IMG_ERR                 1

namespace GUI {

class Image : public Window {
public:
    Image(int x, int y, int width, int height,
          WindowFlags flags = WindowFlags::WNONE,
          WindowPriority priority = WindowPriority::WPRIO0);
    ~Image();

    int loadImage(const char *path);
    uint32_t *getBuff(void);
private:
    int imgFd;
    uint32_t *imgBuff;
};

}