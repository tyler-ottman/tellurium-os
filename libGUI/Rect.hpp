#pragma once

#include <stddef.h>

namespace GUI {

#define MAX_SPLIT                   4

class Rect {
public:
    Rect(int top, int bottom, int left, int right);
    Rect(void);
    ~Rect();
    Rect(Rect &rect);

    bool intersect(Rect *res, Rect *rect);
    bool intersects(Rect *rect);
    bool contains(Rect *rect);
    int split(Rect *splitRects, Rect *crossRect);

    int getLeft(void);
    int getRight(void);
    int getTop(void);
    int getBottom(void);

    void setLeft(int left);
    void setRight(int right);
    void setTop(int top);
    void setBottom(int bottom);

private:
    int top;
    int bottom;
    int left;
    int right;
};

}