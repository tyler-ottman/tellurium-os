#ifndef LIMINE_FB
#define LIMINE_FB

#include <stdint.h>

uint32_t fb_get_width(void);
uint32_t fb_get_height(void);
uint32_t *fb_get_framebuffer(void);
uint16_t fb_get_bpp(void);
uint64_t fb_get_pitch(void);

void init_framebuffer(void);

#endif // LIMINE_FB