#ifndef IMAGEREADER_H
#define IMAGEREADER_H

#include "DataStream.hpp"
#include <stdint.h>

/// @brief Possible errors to encounter while opening image
enum ImageReaderErrors {
    ERR_NOERR                               = 0,
    ERR_DEFAULT                             = 1,
    ERR_INVALID_FILE_FORMAT                 = 2,
    ERR_INVALID_ARGS                        = 3,
    ERR_INVALID_BPP                         = 4,
    ERR_INVALID_DIMENSIONS                  = 5,
    ERR_UNSUPPORTED_COMPRESS                = 6,
    ERR_INVALID_ID                          = 7,
    ERR_INVALID_DIB_HEADER                  = 8,
    ERR_INVALID_COLOR_PLANES                = 9,
    ERR_INVALID_COLOR_PALETTE               = 10,
    ERR_INVALID_IMPORTANT_COLORS            = 11,
    ERR_NO_MEM                              = 12
};

class ImageReader {
public:
    ImageReader(const char *path);
    virtual ~ImageReader(void);

    /// @brief Given image path, load image to memory
    /// @param path The path image // Support more than ppm
    /// @return 
    virtual ImageReaderErrors loadImage(const char *path) = 0;

    /// @brief Get width of image
    int getWidth(void);

    /// @brief Get height of image
    int getHeight(void);

    /// @brief Get bits per pixel
    int getBpp(void);

    /// @brief Get image buffer
    uint32_t *getBuff(void);

protected:
    int imgFd;
    int width;
    int height;
    int bpp;
    uint32_t *buff;
};

/// @brief ImageReader driver returns reader for specific file format
/// @param path The file name of the image
/// @return Driver status
ImageReader *imageReaderDriver(const char *path);

class PpmReader : public ImageReader {
public:
    PpmReader(const char *path);

    /// @brief Load Ppm image format
    virtual ImageReaderErrors loadImage(const char *path);

private:
    int getPpmNumber(uint8_t *ppmMeta);
};

class BmpReader : public ImageReader {
public:
    BmpReader(const char *path);
    ~BmpReader(void);

    /// @brief Load Bmp image format
    virtual ImageReaderErrors loadImage(const char *path);

private:
    enum CompressionMethod {
        BI_RGB                  = 0,
        BI_RLE8                 = 1,
        BI_RLE4                 = 2,
        BI_BITFIELDS            = 3,
        BI_JPEG                 = 4,
        BI_PNG                  = 5,
        BI_ALPHABITFIELDS       = 6,
        BI_CMYK                 = 11,
        BI_CMYKRLE8             = 12,
        BI_CMYKRLE4             = 13,
    };

    struct BmpFileHeader {
        uint16_t headerID;                          // ID of BMP file
        uint32_t fileSize;                          // Size of file in bytes
        uint16_t reserved0;
        uint16_t resorved1;
        uint32_t bmpOffset;                         // Byte offset of start of bitmap image data
    };

    struct DibHeader {
        uint32_t dibHeaderLen;                      // Length of DIB Header
        uint32_t bmpWidth;                          // Width of bitmap in pixels
        uint32_t bmpHeight;                         // Height of bitmap in pixels
        uint16_t nColorPlanes;                      // Number of Color Planes (always 1)
        uint16_t bpp;                               // Bits per pixel
        uint32_t compressionMethod;                 // CompressionMethod
        uint32_t imageSize;                         // 0 for BI_RGB
        uint32_t hResolution;                       // Horizontal Resolution (pixels per meter)
        uint32_t vResolution;                       // Vertical Resolution (pixels per meter)
        uint32_t nColorPalette;                     // Number of Color Palettes (0 for 2^n)
        uint32_t nImportantColors;                  // Number of important colors (0 if all important)
    };

    /// @brief Get the nth bit starting from data
    uint8_t bit(uint8_t *data, size_t n);

    /// @brief Get the pixel at (row, column) as 32 bit ARGB format
    uint32_t bmpGetPixel(uint8_t *data, size_t row, size_t column);

    /// @brief Verify the integrity of BMP file header
    /// @param bmpStream The data stream pointing to the beginning of the header file
    /// @return Upon success, 0, otherwise an error code
    ImageReaderErrors loadBmpHeaders(DataStream *bmpStream);

    BmpFileHeader *bmpHeader;
    DibHeader *dibHeader;
};

#endif // IMAGEREADER_H