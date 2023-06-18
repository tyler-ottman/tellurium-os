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

    bool isIntersect(Rect *rect);
    int split(Rect *splitRects, Rect *crossRect);
    void reset(void);

    int getLeft(void);
    int getRight(void);
    int getTop(void);
    int getBottom(void);

    void setLeft(int left);
    void setRight(int right);
    void setTop(int top);
    void setBottom(int bottom);

    void setUsed(void);
    bool isFree(void);

private:
    int top;
    int bottom;
    int left;
    int right;
    bool free;
};

}