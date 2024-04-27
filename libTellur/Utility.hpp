#ifndef UTILITY_H
#define UTILITY_H

#include <stdint.h>

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

struct vec2 {
    vec2(void) : x(0), y(0) {}
    vec2(int x, int y) : x(x), y(y) {}
    bool equals(vec2 &other) { return x == other.x && y == other.y; }
    bool equals(vec2 *other) { return x == other->x && y == other->y; }
    int x;
    int y;
};

uint32_t translateLightColor(uint32_t color);
bool isWhitespace(char c);
int getNumDigits(int num);

#endif // UTILITY_H