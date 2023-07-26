#include "Image.hpp"
#include "ulibc/string.h"

namespace GUI {

Image::Image(int x, int y, int width, int height)
    : Window::Window("image", x, y, width, height, 0), imgFd(-1) {
    priority = 0;
    type = GUI::WindowImage;
}

Image::~Image() {}

int Image::loadImage(const char *path) {
    if (imgFd == -1) {
        imgFd = syscall_open(path, 0);
    }
    
    if (imgFd == -1) {
        return IMG_ERR;
    }
    
    imgBuff = new uint32_t[width * height];
    
    if (!imgBuff) {
        return IMG_ERR;
    }
    
    syscall_read(imgFd, imgBuff, 4 * width * height);

    return IMG_OK;
}

void Image::drawImage() {
    context->drawBuff(x, y, width, height, imgBuff);
}

}  // namespace GUI