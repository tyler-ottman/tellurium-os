#ifndef UTILITY_H
#define UTILITY_H

struct vec2 {
    vec2(void) : x(0), y(0) {}
    vec2(int x, int y) : x(x), y(y) {}
    bool equals(vec2 &other) { return x == other.x && y == other.y; }
    bool equals(vec2 *other) { return x == other->x && y == other->y; }
    int x;
    int y;
};

#endif // UTILITY_H