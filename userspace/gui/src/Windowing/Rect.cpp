#include "Windowing/Rect.hpp"

namespace GUI {

Rect::Rect(int top, int bottom, int left, int right)
    : top(top), bottom(bottom), left(left), right(right), free(false) {}

Rect::Rect() : free(false) {

}

Rect::~Rect() {

}

Rect::Rect(Rect &rect) {
    this->top = rect.getTop();
    this->bottom = rect.getBottom();
    this->left = rect.getLeft();
    this->right = rect.getRight();
    this->free = false;
}

bool Rect::isIntersect(Rect *rect) {
    return (left <= rect->getRight() && right >= rect->getLeft() &&
            top <= rect->getBottom() && bottom >= rect->getTop());
}

int Rect::split(Rect *splitRects, Rect *cutRect) {
    int numRegions = 0;
    Rect thisCopy(top, bottom, left, right);

    if (getLeft() > cutRect->getLeft() &&
        getLeft() <= cutRect->getRight()) {
        Rect clippedRegion(thisCopy.getTop(), thisCopy.getBottom(),
                           thisCopy.getLeft(), cutRect->getLeft() - 1);

        thisCopy.setLeft(cutRect->getLeft());

        splitRects[numRegions++] = clippedRegion;
    }

    if (cutRect->getTop() >= thisCopy.getTop() &&
        cutRect->getTop() <= thisCopy.getBottom()) {
        Rect clippedRegion(thisCopy.getTop(), cutRect->getTop() - 1,
                           thisCopy.getLeft(), thisCopy.getRight());

        thisCopy.setTop(cutRect->getTop());

        splitRects[numRegions++] = clippedRegion;
    }

    if (cutRect->getRight() >= thisCopy.getLeft() &&
        cutRect->getRight() <= thisCopy.getRight()) {
        Rect clippedRegion(thisCopy.getTop(), thisCopy.getBottom(),
                           cutRect->getRight() + 1, thisCopy.getRight());

        thisCopy.setRight(cutRect->getRight());

        splitRects[numRegions++] = clippedRegion;
    }

    if (cutRect->getBottom() >= thisCopy.getTop() &&
        cutRect->getBottom() <= thisCopy.getBottom()) {
        Rect clippedRegion(cutRect->getBottom() + 1, thisCopy.getBottom(),
                           thisCopy.getLeft(), thisCopy.getRight());

        thisCopy.setBottom(cutRect->getBottom());

        splitRects[numRegions++] = clippedRegion;
    }

    return numRegions;
}

void Rect::reset() {
    free = true;
}

int Rect::getLeft() {
    return left;
}

int Rect::getRight() {
    return right;
}

int Rect::getTop() {
    return top;
}

int Rect::getBottom() {
    return bottom;
}

void Rect::setLeft(int left) {
    this->left = left;
}

void Rect::setRight(int right) {
    this->right = right;
}

void Rect::setTop(int top) {
    this->top = top;
}

void Rect::setBottom(int bottom) {
    this->bottom = bottom;
}

bool Rect::isFree() {
    return free;
}

}