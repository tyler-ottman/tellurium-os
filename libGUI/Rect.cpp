#include "Rect.hpp"

namespace GUI {

Rect::Rect(int x, int y, int width, int height)
    : x(x), y(y), width(width), height(height) {}

Rect::Rect(Rect &rect) {
    x = rect.getLeft();
    y = rect.getTop();
    width = rect.getWidth();
    height = rect.getHeight();
}

Rect::Rect() {}

Rect::~Rect() {}

bool Rect::getOverlapRect(Rect *res, Rect *rect) {
    if (!(intersects(rect))) {
        return false;
    }

    *res = *this;

    if (rect->getLeft() >= res->getLeft() &&
        rect->getLeft() <= res->getRight()) {
        res->setLeft(rect->getLeft());
    }

    if (rect->getTop() >= res->getTop() && rect->getTop() <= res->getBottom()) {
        res->setTop(rect->getTop());
    }

    if (rect->getRight() >= res->getLeft() &&
        rect->getRight() <= res->getRight()) {
        res->setRight(rect->getRight());
    }

    if (rect->getBottom() >= res->getTop() &&
        rect->getBottom() <= res->getBottom()) {
        res->setBottom(rect->getBottom());
    }

    return true;
}

int Rect::getSplitRects(Rect *splitRects, Rect *cutRect) {
    int numRegions = 0;
    Rect thisCopy = *this;

    if (cutRect->getLeft() >= thisCopy.getLeft() &&
        cutRect->getLeft() <= thisCopy.getRight()) {
        int newWidth = cutRect->getLeft() - thisCopy.getLeft();
        Rect clippedRegion(thisCopy.getX(), thisCopy.getY(), newWidth,
                           thisCopy.getHeight());

        thisCopy.setLeft(cutRect->getLeft());

        splitRects[numRegions++] = clippedRegion;
    }

    if (cutRect->getTop() >= thisCopy.getTop() &&
        cutRect->getTop() <= thisCopy.getBottom()) {
        int newHeight = cutRect->getTop() - thisCopy.getTop();
        Rect clippedRegion(thisCopy.getX(), thisCopy.getY(),
                           thisCopy.getWidth(), newHeight);

        thisCopy.setTop(cutRect->getTop());

        splitRects[numRegions++] = clippedRegion;
    }

    if (cutRect->getRight() >= thisCopy.getLeft() &&
        cutRect->getRight() <= thisCopy.getRight()) {
        int newX = cutRect->getRight() + 1;
        int newWidth = thisCopy.getRight() - newX + 1;
        Rect clippedRegion(newX, thisCopy.getY(), newWidth,
                           thisCopy.getHeight());

        thisCopy.setRight(cutRect->getRight());

        splitRects[numRegions++] = clippedRegion;
    }

    if (cutRect->getBottom() >= thisCopy.getTop() &&
        cutRect->getBottom() <= thisCopy.getBottom()) {
        int newY = cutRect->getBottom() + 1;
        int newHeight = thisCopy.getBottom() - newY + 1;
        Rect clippedRegion(thisCopy.getX(), newY, thisCopy.getWidth(),
                           newHeight);

        thisCopy.setBottom(cutRect->getBottom());

        splitRects[numRegions++] = clippedRegion;
    }

    return numRegions;
}

bool Rect::intersects(Rect *rect) {
    return getLeft() <= rect->getRight() && getRight() >= rect->getLeft() &&
           getTop() <= rect->getBottom() && getBottom() >= rect->getTop();
}

bool Rect::contains(Rect *rect) {
    return getTop() <= rect->getTop() && getBottom() >= rect->getBottom() &&
           getLeft() <= rect->getLeft() && getRight() >= rect->getRight();
}

int Rect::getX() { return x; }

int Rect::getY() { return y; }

int Rect::getWidth() { return width; }

int Rect::getHeight() { return height; }

int Rect::getLeft() { return x; }

int Rect::getRight() { return x + width - 1; }

int Rect::getTop() { return y; }

int Rect::getBottom() { return y + height - 1; }

void Rect::setX(int xPos) { x = xPos; }

void Rect::setY(int yPos) { y = yPos; }

void Rect::setWidth(int width) { this->width = width; }

void Rect::setHeight(int height) { this->height = height; }

void Rect::setLeft(int left) {
    // if (left > getRight()) {
    //     return;
    // }

    // DeltaX = left - x
    // New width = old width - DeltaX
    width -= left - x;

    x = left;
}

void Rect::setRight(int right) {
    // if (right < getLeft()) {
    //     return;
    // }

    // New width = right - x + 1
    width = right - x + 1;
}

void Rect::setTop(int top) {
    // if (top > getBottom()) {
    //     return;
    // }
    
    // DeltaY = top - y
    // New height = old height + DeltaY
    height -= top - y;

    y = top;
}

void Rect::setBottom(int bottom) {
    // if (bottom < getTop()) {
    //     return;
    // }

    // New height = bottom - y + 1
    height = bottom - y + 1;
}

void Rect::setPosition(int xPos, int yPos) {
    x = xPos;
    y = yPos;
}

bool Rect::equals(Rect &other) {
    return x == other.x && y == other.y && width == other.width &&
           height == other.height;
}

bool Rect::equals(Rect *other) {
    return x == other->x && y == other->y && width == other->width &&
           height == other->height;
}

}
