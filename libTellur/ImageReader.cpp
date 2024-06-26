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
    } else if (!__strncmp(fileType, ".bmp", maxLen)) {
        return new BmpReader(path);
    }
    
    // Unsupported file format
    return nullptr;
}


// PPM File Format

PpmReader::PpmReader(const char *path) : ImageReader(path) {
    bpp = 32;

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

ImageReaderErrors PpmReader::loadImage(const char *path) {
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

// BMP File Format

BmpReader::BmpReader(const char *path) : ImageReader(path) {
    bpp = 32;

    bmpHeader = new BmpFileHeader;
    dibHeader = new DibHeader;

    loadImage(path); // TODO: Fix and move out of constructor
}

BmpReader::~BmpReader() {}

ImageReaderErrors BmpReader::loadImage(const char *path) {
    // Open BMP File for reading
    imgFd = syscall_open(path, 0);
    if (imgFd == -1) { return ERR_DEFAULT; }

    // Read BMP file into memory
    size_t headerInfoLen = sizeof(BmpFileHeader) + sizeof(DibHeader);
    uint8_t *headerData = new uint8_t[headerInfoLen];
    if (!headerData) { return ERR_NO_MEM; }
    syscall_read(imgFd, headerData, headerInfoLen);

    // Validate BMP Header file information
    DataStream bmpStream(headerData);
    ImageReaderErrors err = loadBmpHeaders(&bmpStream);
    if (err) { return err; }
    
    // Load BMP Image data to memory
    uint8_t *bmpData = new uint8_t[dibHeader->imageSize];
    if (!bmpData) { return ERR_NO_MEM; }
    syscall_lseek(imgFd, bmpHeader->bmpOffset, 0);
    syscall_read(imgFd, bmpData, dibHeader->imageSize);
    bmpStream = DataStream(bmpData);

    // Copy BMP file to buff
    int bmpWidth = dibHeader->bmpWidth;
    int bmpHeight = dibHeader->bmpHeight;
    buff = new uint32_t[bmpWidth * bmpHeight];
    if (!buff) { return ERR_NO_MEM; }

    bmpStream.setOffset(bmpHeader->bmpOffset); // Point to beginning of image data
    uint8_t *data = (uint8_t *)bmpStream.getData();
    size_t pixelCounter = 0;
    for (int i = bmpHeight - 1; i >= 0; i--) {
        for (int j = 0; j < bmpWidth; j++) {
            // Get the pixel at row i, column j
            buff[pixelCounter++] = bmpGetPixel(data, i, j);
        }
    }

    width = dibHeader->bmpWidth;
    height = dibHeader->bmpHeight;
    bpp = dibHeader->bpp;

    delete[] headerData;
    delete[] bmpData;

    return ERR_NOERR;
}

uint8_t BmpReader::bit(uint8_t *data, size_t n) {
    return (data[n / 8] >> (n % 8)) & 1;
}

// Convert the data into a 32 bpp ARGB pixel
uint32_t BmpReader::bmpGetPixel(uint8_t *data, size_t row, size_t column) {
    uint8_t alpha = 0;
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;

    // Bit offset from image data of pixel (row, column)
    size_t bitStart = dibHeader->bpp * (dibHeader->bmpWidth * row + column);

    switch (dibHeader->bpp) {
    case 32: {
        for (int i = 0; i < 8; i++) {
            alpha |= (bit(data, bitStart + 24 + i) << i);
            red |= (bit(data, bitStart + 16 + i) << i);
            green |= (bit(data, bitStart + 8 + i) << i);
            blue |= (bit(data, bitStart + i) << i);
        }
        break;
    }
        
    case 24:
        for (int i = 0; i < 8; i++) {
            red |= (bit(data, bitStart + 16 + i) << i);
            green |= (bit(data, bitStart + 8 + i) << i);
            blue |= (bit(data, bitStart + i) << i);
        }
        break;
    }

    return (alpha << 24) | (red << 16) | (green << 8) | blue;
}

ImageReaderErrors BmpReader::loadBmpHeaders(DataStream *bmpStream) {
    // Load Bitmap File Header
    bmpHeader->headerID = bmpStream->decodeUInt16();
    if (bmpHeader->headerID != 0x4d42) { // BM, Windows 3.1x, 95, NT, ... etc
        return ImageReaderErrors::ERR_INVALID_ID;
    }

    bmpHeader->fileSize = bmpStream->decodeUInt32();
    bmpHeader->reserved0 = bmpStream->decodeUInt16();
    bmpHeader->resorved1 = bmpStream->decodeUInt16();
    bmpHeader->bmpOffset = bmpStream->decodeUInt32();

    // Load Dib Header
    dibHeader->dibHeaderLen = bmpStream->decodeUInt32();
    if (dibHeader->dibHeaderLen != 124) { // Windows NT 5.0, 98 or later
        return ImageReaderErrors::ERR_INVALID_DIB_HEADER;
    }
    dibHeader->bmpWidth = bmpStream->decodeUInt32();
    dibHeader->bmpHeight = bmpStream->decodeUInt32();
    dibHeader->nColorPlanes = bmpStream->decodeUInt16();
    if (dibHeader->nColorPlanes != 1) { // Must have exactly 1 color plane
        return ImageReaderErrors::ERR_INVALID_COLOR_PLANES;
    }
    dibHeader->bpp = bmpStream->decodeUInt16();
    if (dibHeader->bpp != 32 && dibHeader->bpp != 24) {
        return ImageReaderErrors::ERR_INVALID_BPP;
    }
    dibHeader->compressionMethod = bmpStream->decodeUInt32();
    if (dibHeader->compressionMethod != CompressionMethod::BI_RGB &&
        dibHeader->compressionMethod != CompressionMethod::BI_BITFIELDS) {
        return ImageReaderErrors::ERR_UNSUPPORTED_COMPRESS;
    }
    dibHeader->imageSize = bmpStream->decodeUInt32();
    if ((8 *dibHeader->imageSize) != (dibHeader->bpp * dibHeader->bmpHeight * dibHeader->bmpWidth)) {
        return ImageReaderErrors::ERR_INVALID_DIMENSIONS;
    }
    dibHeader->hResolution = bmpStream->decodeUInt32();
    dibHeader->vResolution = bmpStream->decodeUInt32();
    dibHeader->nColorPalette = bmpStream->decodeUInt32();
    if (dibHeader->nColorPalette != 0) { // Should default to 0 (2^32)
        return ImageReaderErrors::ERR_INVALID_COLOR_PALETTE;
    }
    dibHeader->nImportantColors = bmpStream->decodeUInt32();
    if (dibHeader->nImportantColors != 0) { // 0 because every color required
        return ImageReaderErrors::ERR_INVALID_IMPORTANT_COLORS;
    }

    return ImageReaderErrors::ERR_NOERR;
}
