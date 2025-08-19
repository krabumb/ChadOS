#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <stdint.h>

void init_graphics(void *mb_info);
void draw_pixel(int x, int y, uint32_t argb); // 0xAARRGGBB
void fill_screen(uint32_t argb);
void test_pattern(void);

// Optional getters for debugging
uint32_t fb_width(void);
uint32_t fb_height(void);
uint32_t fb_bpp(void);
uint64_t fb_addr(void);
uint32_t fb_pitch(void);

#endif // GRAPHICS_H
