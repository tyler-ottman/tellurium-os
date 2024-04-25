#ifndef SURFACE_H
#define SURFACE_H

#include "Rect.hpp"
#include <stdint.h>

namespace GUI {

struct Surface {
    Surface(Rect& rect, uint32_t *buff) : rect(rect), buff(buff) {}
    Surface(void) : buff(nullptr) {}

    /// @brief Alpha Blit surface under this
    /// @param surface The surface source
    /// @param area The area to restrict the alpha blitting to
    void alphaBlit(Surface& source, Rect *area);

    /// @brief Blit source surface to destination surface (this)
    /// @param source The surface source
    /// @param area The area to restrict the blitting to
    void blit(Surface& source, Rect *area);

    /// @brief Get the number of pixels of a surface
    size_t getSize(void);

    Rect rect; // The rectangular bounds of the surface
    uint32_t *buff; // The surface's buffer
};

}

#endif // SURFACE_H