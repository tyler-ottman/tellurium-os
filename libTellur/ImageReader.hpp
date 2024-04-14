#ifndef IMAGEREADER_H
#define IMAGEREADER_H

#include <stdint.h>

enum ImageReaderErrors {
    ERR_NOERR = 0,
    ERR_DEFAULT = 1
};

class ImageReader {
public:
    ImageReader(const char *path);
    virtual ~ImageReader(void);

    /// @brief Given image path, load image to memory
    /// @param path The path image // Support more than ppm
    /// @return 
    virtual int loadImage(const char *path) = 0;

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

class PpmReader : public ImageReader {
public:
    PpmReader(const char *path);

    /// @brief Load Ppm image format
    int loadImage(const char *path) override;

private:
    int getPpmNumber(uint8_t *ppmMeta);
};

#endif // IMAGEREADER_H