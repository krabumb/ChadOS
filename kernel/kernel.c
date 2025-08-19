#include "../src/serial.h"
#include "../src/graphics.h"
#include <stdint.h>

static inline void draw_filled_rect(int x, int y, int w, int h, uint32_t argb) {
    for (int yy = 0; yy < h; ++yy) {
        int py = y + yy;
        for (int xx = 0; xx < w; ++xx) {
            int px = x + xx;
            draw_pixel(px, py, argb);
        }
    }
}

static inline void draw_rect_border(int x, int y, int w, int h, int thickness, uint32_t argb) {
    // top
    draw_filled_rect(x, y, w, thickness, argb);
    // bottom
    draw_filled_rect(x, y + h - thickness, w, thickness, argb);
    // left
    draw_filled_rect(x, y, thickness, h, argb);
    // right
    draw_filled_rect(x + w - thickness, y, thickness, h, argb);
}

__attribute__((used))
void kernel_main(void* mb_info){
    serial_init();
    serial_write("Hello from ChadOS!\n");

    init_graphics(mb_info);
    serial_write("Graphics initialized\n");

    // desktop background
    fill_screen(0xFFFF0000); // 0xAARRGGBB red

    // centered white window
    // We use the getters added in graphics.c (fb_width/fb_height)
    const int screen_w = (int)fb_width();
    const int screen_h = (int)fb_height();

    // Window size (adjust)
    const int win_w = screen_w / 2;      // 50% of the width
    const int win_h = screen_h / 2;      // 50% of the height

    // Centered position
    const int win_x = (screen_w - win_w) / 2;
    const int win_y = (screen_h - win_h) / 2;

    // White fill
    draw_filled_rect(win_x, win_y, win_w, win_h, 0xFFFFFFFF);

    // Light gray title bar of 30 px
    draw_filled_rect(win_x, win_y, win_w, 30, 0xFFDDDDDD);

    // Small black border (2 px)
    draw_rect_border(win_x, win_y, win_w, win_h, 2, 0xFF000000);

    serial_write("Drew red background + centered white window.\n");

    while(1) __asm__("hlt");
}
