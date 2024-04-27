#include "Surface.hpp"

namespace GUI {

// https://en.wikipedia.org/wiki/Alpha_compositing for equation
// The original equation represents the pixel's channels as a decimal, 0.0 - 1.0
// Converting an input channel from a decimal to an 8-bit value, we have
// alphaB' = alphaB * (256 - alphaA) / 256 (no overflow if alphaB' is 8-bit value)
// alphaOut = alphaA + alphaB' (no overflow if alphaOut is 8-bit value)
// pixelOut = (colorA * alphaA + colorB * alphaB) / alphaOut
inline uint32_t alphaBlendPixel(uint32_t colorA, uint32_t colorB) {
    uint8_t alphaA = colorA >> 24; // Pixel A's alpha channel
    uint8_t alphaB = colorB >> 24; // Pixel B's alpha channel

    // Optimization: Do not blend if front pixel is fully opaque
    if (alphaA == 0xff) {
        return colorA;
    }
    
    // Optimization: Do not blend if front pixel is fully transparent
    else if (alphaA == 0x00)  {
        return colorB;
    }

    // The adjusted alpha channel component of Pixel B
    uint8_t alphaB_ = (alphaB * (256 - alphaA)) >> 8;

    // The output alpha channel of blended pixel
    uint8_t alphaOut = alphaA + alphaB_;

    // The output color channels of blended pixel
    uint8_t redOut = (((colorA >> 16) & 0xff) * alphaA + ((colorB >> 16) & 0xff) * alphaB_) / alphaOut;
    uint8_t greenOut = (((colorA >> 8) & 0xff) * alphaA + ((colorB >> 8) & 0xff) * alphaB_) / alphaOut;
    uint8_t blueOut = ((colorA & 0xff) * alphaA + (colorB & 0xff) * alphaB_) / alphaOut;

    return (alphaOut << 24) | (redOut << 16) | (greenOut << 8) | blueOut;
}

void Surface::alphaBlit(Surface &source, Rect *area) {
    Rect drawRect(source.rect);
    int translateX = drawRect.getX();
    int translateY = drawRect.getY();

    // Restrict drawing to area Rect and source Surface
    area->restrictRect(&drawRect);
    rect.restrictRect(&drawRect);    

    // Bounds and offset for drawing
    int x = drawRect.getX();
    int y = drawRect.getY();
    int xMax = drawRect.getRight();
    int yMax = drawRect.getBottom();
    int destWidth = rect.getWidth();
    int sourceWidth = source.rect.getWidth();

    uint32_t *sourceBuff = source.buff;
    for (int i = y; i < yMax; i++) {
        for (int j = x; j < xMax; j++) {
            buff[i * destWidth + j] = alphaBlendPixel(buff[i * destWidth + j], sourceBuff[(i - translateY) * sourceWidth + (j - translateX)]);
        }
    }
}

void Surface::blit(Surface &source, Rect *area) {
    Rect drawRect(source.rect);
    int translateX = drawRect.getX();
    int translateY = drawRect.getY();

    // Restrict drawing to area Rect and source Surface
    area->restrictRect(&drawRect);
    rect.restrictRect(&drawRect);    

    // Bounds and offset for drawing
    int x = drawRect.getX();
    int y = drawRect.getY();
    int xMax = drawRect.getRight();
    int yMax = drawRect.getBottom();
    int destWidth = rect.getWidth();
    int sourceWidth = source.rect.getWidth();

    uint32_t *sourceBuff = source.buff;
    for (int i = y; i < yMax; i++) {
        for (int j = x; j < xMax; j++) {
            buff[i * destWidth + j] = sourceBuff[(i - translateY) * sourceWidth + (j - translateX)];
        }
    }
}

size_t Surface::getSize(void) { return rect.getWidth() * rect.getHeight(); }

}
