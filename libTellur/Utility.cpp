#include "Utility.hpp"

uint32_t translateLightColor(uint32_t color) {
    uint8_t red = (color >> 16) & 0xff;
    uint8_t green = (color >> 8) & 0xff;
    uint8_t blue = color & 0xff;

    uint8_t deltaRed = MIN(red + (red / 8), 0xff);
    uint8_t deltaGreen = MIN(green + (green / 8), 0xff);
    uint8_t deltaBlue = MIN(blue + (blue / 8), 0xff);

    return 0xff000000 | (deltaRed << 16) | (deltaGreen << 8) | (deltaBlue);
}

bool isWhitespace(char c) {
    return c < 0x30 || c > 0x39;
}

int getNumDigits(int num) {
    int count = 1;
    
    while (num / 10 != 0) {
        count++;
        num /= 10;
    }

    return count;
}
