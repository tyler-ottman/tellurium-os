#include "Image.hpp"
#include "ulibc/stdlib.h"
#include "ulibc/string.h"

#define MAX_FIELD_LEN                   10

static inline bool isWhitespace(char c) {
    return c == 0x20 || c == 0x09 || c == 0x0d || c == 0x0a;
}

static inline int getPpmNumber(uint8_t *ppmMeta) {
    char buff[MAX_FIELD_LEN] = {0};
    int i;

    for (i = 0; i < MAX_FIELD_LEN && !isWhitespace(*ppmMeta); i++) {
        buff[i] = *ppmMeta++;
    }
    buff[i] = '\0';
    
    return __atoi(buff);
}

static inline int getNumDigits(int num) {
    int count = 1;
    
    while (num / 10 != 0) {
        count++;
        num /= 10;
    }

    return count;
}

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
    
    // Read PPM header
    uint8_t *ppm = new uint8_t[100];
    syscall_read(imgFd, ppm, 100);
    uint8_t *ppmMeta = ppm;

    // Verify magic/whitespace
    if (*ppmMeta++ != 'P' || *ppmMeta++ != '6' || !isWhitespace(*ppmMeta++)) {
        return IMG_ERR;
    }

    // Get width
    int width = getPpmNumber(ppmMeta);
    ppmMeta += getNumDigits(width);

    // Verify whitespace
    if (!isWhitespace(*ppmMeta++)) {
        return IMG_ERR;
    }

    // Get height
    int height = getPpmNumber(ppmMeta);
    ppmMeta += getNumDigits(height);

    // Verify whitespace
    if (!isWhitespace(*ppmMeta++)) {
        return IMG_ERR;
    }

    // Get max color value
    int maxValue = getPpmNumber(ppmMeta);
    ppmMeta += getNumDigits(maxValue);

    // Verify whitespace
    if (!isWhitespace(*ppmMeta++)) {
        return IMG_ERR;
    }

    int headerLen = (int)(ppmMeta - ppm);

    // Read pixels
    int fileSize = headerLen + 3 * width * height;
    delete[] ppm;
    ppm = new uint8_t[fileSize];
    if (!ppm) {
        return IMG_ERR;
    }

    // Now read entire file
    syscall_lseek(imgFd, 0, 0);
    syscall_read(imgFd, ppm, fileSize);
    ppm += headerLen;

    // Copy PPM pixels to 32-bit image buffer
    imgBuff = new uint32_t[width * height];
    if (!imgBuff) {
        return IMG_ERR;
    }

    for (int i = 0; i < width * height; i++) {
        imgBuff[i] = 0xff000000 | (ppm[0] << 16) | (ppm[1] << 8) | (ppm[2]);
        ppm += 3;
    }

    delete ppm;

    return IMG_OK;
}

void Image::drawImage() {
    context->drawBuff(x, y, width, height, imgBuff);
}

}  // namespace GUI