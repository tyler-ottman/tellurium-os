#include "ImageReader.hpp"
#include "flibc/stdlib.h"
#include "flibc/string.h"
#include "syscalls.hpp"
#include "Utility.hpp"

ImageReader::ImageReader(const char *path)
    : imgFd(-1), width(0), height(0), bpp(0), buff(nullptr) {}

ImageReader::~ImageReader() {
    // Close the image file
    if (imgFd != -1) {
        // syscall_close(imgFd);
    }

    // Delete the buffer used to load the image
    delete buff;
}

int ImageReader::getWidth() { return width; }

int ImageReader::getHeight() { return height; }

int ImageReader::getBpp() { return bpp; }

uint32_t *ImageReader::getBuff() { return buff; }
ImageReader *imageReaderDriver(const char *path) {
    if (!path) {
        return nullptr;
    }

    const char *fileType = nullptr;
    for (int i = __strlen(path) - 1; i >= 0; i--) {
        if (path[i] == '.') {
            fileType = &path[i];
            break;
        }
    }
    
    // If file extension not found, return error
    if (!fileType) {
        return nullptr;
    }

    int maxLen = __strlen(fileType);
    if (!__strncmp(fileType, ".ppm", maxLen)) {
        return new PpmReader(path);
    }
    
    // Unsupported file format
    return nullptr;
}

PpmReader::PpmReader(const char *path) : ImageReader(path) {
    bpp = 4;

    loadImage(path);
}

int PpmReader::getPpmNumber(uint8_t *ppmMeta) {
    char str[10];
    
    int i;
    for (i = 0; i < 10 && !isWhitespace(*ppmMeta); i++) {
        str[i] = *ppmMeta++;
    }
    str[i] = '\0';
    
    return __atoi(str);
}

int PpmReader::loadImage(const char *path) {
    // Open PPM File for reading
    imgFd = syscall_open(path, 0);
    if (imgFd == -1) { return ERR_DEFAULT; }
    

    // Read PPM header (TODO: encapsulate reading data of discrete sizes)
    uint8_t *ppm = new uint8_t[100];
    syscall_read(imgFd, ppm, 100);
    uint8_t *ppmMeta = ppm; // Verify magic/whitespace
    if (*ppmMeta++ != 'P' || *ppmMeta++ != '6') { return ERR_DEFAULT; }
    while (isWhitespace(*ppmMeta)) { ppmMeta++; }
    width = getPpmNumber(ppmMeta);
    ppmMeta += getNumDigits(getWidth()); // Verify whitespace
    if (!isWhitespace(*ppmMeta++)) { return ERR_DEFAULT; }
    height = getPpmNumber(ppmMeta);
    ppmMeta += getNumDigits(getHeight()); // Verify whitespace
    if (!isWhitespace(*ppmMeta++)) { return ERR_DEFAULT; }
    int maxValue = getPpmNumber(ppmMeta); // Get max color value
    ppmMeta += getNumDigits(maxValue); // Verify whitespace
    if (!isWhitespace(*ppmMeta++)) { return ERR_DEFAULT; }


    // Now read entire file
    int headerLen = (int)(ppmMeta - ppm);
    delete []ppm;
    int fileSize = headerLen + 3 * width * height;
    ppm = new uint8_t[fileSize];
    if (!ppm) { return ERR_DEFAULT; }
    syscall_lseek(imgFd, 0, 0);
    syscall_read(imgFd, ppm, fileSize);
    ppm += headerLen;


    // Copy PPM pixels to Window buffer
    buff = new uint32_t[width * height];
    if (!buff) { return ERR_DEFAULT; }
    for (int i = 0; i < width * height; i++) {
        buff[i] = 0xff000000 | (ppm[0] << 16) | (ppm[1] << 8) | (ppm[2]);
        ppm += 3;
    }
    delete ppm;

    return ERR_NOERR;
}