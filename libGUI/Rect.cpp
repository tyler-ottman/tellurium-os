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

void Rect::operator=(const Rect& other) {
    x = other.x;
    y = other.y;
    width = other.width;
    height = other.height;
}

bool Rect::getOverlapRect(Rect *res, Rect *rect) {
    *res = *rect;

    if (res->getLeft() < getLeft()) {
        res->setLeft(getLeft());
    }

    if (res->getRight() > getRight()) {
        res->setRight(getRight());
    }

    if (res->getTop() < getTop()) {
        res->setTop(getTop());
    }

    if (res->getBottom() > getBottom()) {
        res->setBottom(getBottom());
    }

    return true;
}

int Rect::getSplitRects(Rect *splitRects, Rect *cutRect) {
    int numRegions = 0;
    Rect thisCopy = *this;

    if (cutRect->getLeft() > thisCopy.getLeft() &&
        cutRect->getLeft() < thisCopy.getRight()) {
        int newWidth = cutRect->getLeft() - thisCopy.getLeft();
        Rect clippedRegion(thisCopy.getX(), thisCopy.getY(), newWidth,
                           thisCopy.getHeight());

        thisCopy.setLeft(cutRect->getLeft());

        splitRects[numRegions++] = clippedRegion;
    }

    if (cutRect->getTop() > thisCopy.getTop() &&
        cutRect->getTop() < thisCopy.getBottom()) {
        int newHeight = cutRect->getTop() - thisCopy.getTop();
        Rect clippedRegion(thisCopy.getX(), thisCopy.getY(),
                           thisCopy.getWidth(), newHeight);

        thisCopy.setTop(cutRect->getTop());

        splitRects[numRegions++] = clippedRegion;
    }

    if (cutRect->getRight() > thisCopy.getLeft() &&
        cutRect->getRight() < thisCopy.getRight()) {
        int newX = cutRect->getRight();
        int newWidth = thisCopy.getRight() - newX;
        Rect clippedRegion(newX, thisCopy.getY(), newWidth,
                           thisCopy.getHeight());

        thisCopy.setRight(cutRect->getRight());

        splitRects[numRegions++] = clippedRegion;
    }

    if (cutRect->getBottom() > thisCopy.getTop() &&
        cutRect->getBottom() < thisCopy.getBottom()) {
        int newY = cutRect->getBottom();
        int newHeight = thisCopy.getBottom() - newY;
        Rect clippedRegion(thisCopy.getX(), newY, thisCopy.getWidth(),
                           newHeight);

        thisCopy.setBottom(cutRect->getBottom());

        splitRects[numRegions++] = clippedRegion;
    }

    return numRegions;
}

void Rect::restrictRect(Rect *other) {
    if (other->getTop() < getTop()) {
        other->setTop(getTop());
    }

    if (other->getBottom() > getBottom()) {
        other->setBottom(getBottom());
    }

    if (other->getLeft() < getLeft()) {
        other->setLeft(getLeft());
    }

    if (other->getRight() > getRight()) {
        other->setRight(getRight());
    }
}

bool Rect::overlaps(Rect *rect) {
    return getLeft() < rect->getRight() && getRight() > rect->getLeft() &&
           getTop() < rect->getBottom() && getBottom() > rect->getTop();
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

int Rect::getRight() { return x + width; }

int Rect::getTop() { return y; }

int Rect::getBottom() { return y + height; }

void Rect::setX(int xPos) { x = xPos; }

void Rect::setY(int yPos) { y = yPos; }

void Rect::setWidth(int width) { this->width = width; }

void Rect::setHeight(int height) { this->height = height; }

void Rect::setLeft(int left) {
    // newWidth = oldWidth + (oldLeft - newLeft)
    //          = width + (x - left)
    width += x - left;
    x = left;
}

void Rect::setRight(int right) {
    // newWidth = oldWidth + (newRight - oldRight)
    //          = width + (right - (x + width))
    //          = right - x
    width = right - x;
}

void Rect::setTop(int top) {
    // newHeight = oldHeight + (oldTop - newTop)
    //           = height + (y - top)
    height += (y - top);
    y = top;
}

void Rect::setBottom(int bottom) {
    // newHeight = oldHeight + (newBottom - oldBottom)
    //           = height + (bottom - (y + height))
    //           = bottom - y
    height = bottom - y;
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
