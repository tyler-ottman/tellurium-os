#ifndef RECT_H
#define RECT_H

#include <stddef.h>

namespace GUI {

class Rect {
public:
    // Rect(int top, int bottom, int left, int right);
    /// @brief Rect constructor, takes rectangle dimensions
    /// @param x x-coordinate position of rectangle
    /// @param y y-coordinate position of rectangle
    /// @param width Width of rectangle
    /// @param height Height of rectangle
    Rect(int x, int y, int width, int height);

    /// @brief Copy constructor
    Rect(Rect &rect);

    /// @brief Rect default constructor
    Rect(void);

    /// @brief Destructor
    ~Rect();

    /// @brief Set this Rect to other Rect
    /// @param other The other Rect
    void operator=(const Rect& other);

    /// @brief Get the resulting Rect when overlapping 2 rectangles
    /// @param res Where the resulting Rect is placed
    /// @param rect The rectangle to intersect with
    /// @return True if intersected Rect was created, otherwise false
    bool getOverlapRect(Rect *res, Rect *rect);

    /// @brief Get all the split rectangles when intersecting 2 rectangles
    /// @param res Where the resultings Rects are placed
    /// @param cutRect The rectangle that will cut and divide this
    /// @return The number of rectangles split whewn intersects the 2
    int getSplitRects(Rect *res, Rect *cutRect);

    /// @brief Restrict other Rect dimension to this Rect's dimension
    /// @param other The Rect that will be restricted
    void restrictRect(Rect *other);

    /// @brief Check if rect overlaps with self
    /// @param rect The other rectangle
    /// @return True if this and rect intersect, false otherwise
    bool overlaps(Rect *rect);

    /// @brief Check if rect is contained in self/this
    /// @param rect The Rect to evaluate
    /// @return True if rect is contained in this, false otherwise
    bool contains(Rect *rect);

    /// @brief Get x position of rectangle
    /// @return The x position
    int getX(void);

    /// @brief Get y position of retangle
    /// @return The y position
    int getY(void);

    /// @brief Get width of rectangle
    /// @return The width
    int getWidth(void);

    /// @brief Get height of rectangle
    /// @return The height
    int getHeight(void);

    /// @brief Get left position of rectangle (x-coordinate)
    /// @return The position
    int getLeft(void);

    /// @brief Get right position of rectangle (x-coordinate + width)
    /// @return The position
    int getRight(void);

    /// @brief Get top position of rectangle (y-coordinate)
    /// @return The position
    int getTop(void);

    /// @brief Get bottom position of rectangle (y-coordinate + height)
    /// @return The position
    int getBottom(void);

    /// @brief Set x position of rectangle
    /// @param xPos x-position of rectangle
    void setX(int xPos);

    /// @brief Set y position of rectangle
    /// @param yPos y-position of rectangle
    void setY(int yPos);

    /// @brief Set width of rectangle
    /// @param width The width
    void setWidth(int width);

    /// @brief Set height of rectangle
    /// @param height The height
    void setHeight(int height);

    /// @brief Set left position of rectangle (x-coordinate)
    /// @param left The set position
    void setLeft(int left);

    /// @brief Set right position of rectangle (x-coordinate + width)
    /// @param right The set position
    void setRight(int right);

    /// @brief Set top position of rectangle (y-coordinate)
    /// @param top The set position
    void setTop(int top);

    /// @brief Set bottom position of rectangle (y-coordinate + height)
    /// @param bottom 
    void setBottom(int bottom);

    /// @brief Set (x, y) position of rectangle
    /// @param xPos x-position of rectangle
    /// @param yPos y-position of rectangle
    void setPosition(int xPos, int yPos);

    /// @brief Check if 2 Rects are equal
    /// @param other The other Rect to compare to
    /// @return If they are equal, return true, false otherwise
    bool equals(Rect &other);

    /// @brief Check if 2 Rects are equal
    /// @param other The other Rect to compare to
    /// @return If they are equal, return true, false otherwise
    bool equals(Rect *other);

private:
    int x; ///< x-position
    int y; ///< y-position
    int width; ///< width (x-axis)
    int height; ///< height (y-axis)
};

}

#endif // RECT_H
