#include <arch/limine_fb.h>
#include <limine.h>

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

static struct limine_framebuffer_response *framebuffer_response;
static struct limine_framebuffer *framebuffer;
static uint32_t *px_buffer;

uint32_t fb_get_width() { return framebuffer->width; }

uint32_t fb_get_height() { return framebuffer->height; }

uint32_t *fb_get_framebuffer() { return (uint32_t *)framebuffer->address; }

uint16_t fb_get_bpp() { return framebuffer->bpp; }

uint64_t fb_get_pitch() { return framebuffer->pitch; }

void init_framebuffer() {
    framebuffer_response = framebuffer_request.response;
    framebuffer = framebuffer_response->framebuffers[0];
    px_buffer = (uint32_t *)framebuffer->address;
}