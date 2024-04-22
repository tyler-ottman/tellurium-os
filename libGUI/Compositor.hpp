#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include "Window.hpp"

namespace GUI {

struct ClipRegions;

class Compositor {
public:
    Compositor(Surface *surface);
    ~Compositor(void);

    /// @brief Render to final buffer
    /// @param root Top of Window structure
    /// @param mouse Current position of mouse
    /// @param oldMouse Old position of mouse
    void render(Window *root, Window *mouse);

private:
    /// @brief Generate dirty regions from Window tree structure
    /// @param win Cur traversed window
    /// @param dirtyAncestor Highest ancestor of win marked as dirty
    void createDirtyWindowRegions(Window *win, Window* dirtyAncestor);

    /// @brief Generate dirty regions for the Mouse
    /// @param mouse Current position of mouse
    /// @param oldMouse Old position of mouse
    void createDirtyMouseRegion(Window *mouse);

    /// @brief Draw the Window tree structure
    /// @param win The root window
    void drawWindow(Window *win);

    Surface *fSurface; // The surface that the compositor renders the final image
    Surface *bSurface; // The backbuffer surface (avoid reads from framebuffer)

    ClipRegions *fRenderClips; // Regions copied from the backbuffer surface 
    ClipRegions *bRenderClips; // Regions that require a re-render

    // void drawClippedRegions(void);
};

struct ClipRect {
    ClipRect(void) : win(nullptr) {}
    ClipRect(Rect &rect, Window *win = nullptr)
        : rect(rect), win(win) {}
    ClipRect(Window *win) : win(win) {}

    Rect rect; // Dimensions of clipped rectangle
    Window *win;
};

// TODO: Use stl list or some other data structure
struct ClipRegions {
    ClipRegions() : numClipped(0), maxClipped(200) {
        clippedRects = new ClipRect[maxClipped];
    }
    ~ClipRegions() { delete clippedRects; }

    /// @brief Add Rect to clipped region
    /// @param rect The Rect to add
    void addClippedRegion(Rect *rect, Window *win = nullptr);

    /// @brief Divide existing clipped rects that overlap with rect
    /// @param rect The Rect that will be dividing
    /// @param win 
    void divideClippedRegion(Rect *rect, Window *win = nullptr);

    /// @brief Remove Rect in clipped region
    /// @param rect 
    void removeClippedRegion(Rect *rect);

    /// @brief List operations
    void copyRegion(ClipRegions *other) {
        for (int i = 0; i < other->numClipped; i++) {
            clippedRects[i] = other->clippedRects[i];
        }
        numClipped = other->numClipped;
    }
    void pushRect(ClipRect *rect) {
        if (numClipped < maxClipped) {
            clippedRects[numClipped++] = *rect;
        }
    }
    void deleteRect(int index) {
        if (index >= 0 && index < numClipped) {
            for (int i = index; i < numClipped - 1; i++) {
                clippedRects[i] = clippedRects[i + 1];
            }
            numClipped--;
        }
    }
    void resetClippedList(void) { numClipped = 0; }
    int getNumClipped(void) { return numClipped; }
    ClipRect *getClippedRegion(int i) { return &clippedRects[i]; }

    int numClipped; // Number of clipped Rects in clipped list
    int maxClipped; // Max allowed clipped Rects in clipped list
    ClipRect *clippedRects; // The clipped Rect list
};

};

#endif // COMPOSITOR_H