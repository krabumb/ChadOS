#include "graphics.h"
#include "serial.h"
#include <stdint.h>
#include <stddef.h>

static volatile uint8_t *framebuffer = 0;
static uint32_t pitch = 0, g_width = 0, g_height = 0, g_bpp = 0;
static uint8_t fb_type = 0;
// sane defaults
static uint8_t r_pos=16, r_mask=8, g_pos=8, g_mask=8, b_pos=0, b_mask=8; 

// Multiboot2 structs

typedef struct { uint32_t type, size; } __attribute__((packed)) mb2_tag_t;
#define MB2_TAG_END 0
#define MB2_TAG_FRAMEBUFFER 8

typedef struct {
    // 8
    uint32_t type;
    uint32_t size;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;
    // 0=indexed, 1=RGB, 2=text
    uint8_t  framebuffer_type;
    // *** must be 16-bit ***
    uint16_t reserved;
    union {
        // indexed
        struct {
            uint32_t palette_num_colors;
            // palette follows
        } __attribute__((packed)) indexed;
        // RGB
        struct {
            uint8_t red_field_position;
            uint8_t red_mask_size;
            uint8_t green_field_position;
            uint8_t green_mask_size;
            uint8_t blue_field_position;
            uint8_t blue_mask_size;
        } __attribute__((packed)) rgb;
    } ;
} __attribute__((packed)) mb2_tag_framebuffer_t;

static void log_u32(const char *label, uint32_t v){ serial_write(label); print_hex(v); serial_write("\n"); }
static void log_u64(const char *label, uint64_t v){ serial_write(label); print_hex((uint32_t)(v>>32)); serial_write(""); print_hex((uint32_t)v); serial_write("\n"); }

void init_graphics(void *mb_info){
    uintptr_t addr = (uintptr_t)mb_info;
    uint32_t total = *(uint32_t*)addr; (void)total;
    // skip total_size + reserved
    addr += 8;

    for(;;){
        mb2_tag_t *tag = (mb2_tag_t*)addr;
        if(tag->type == MB2_TAG_END) break;

        if(tag->type == MB2_TAG_FRAMEBUFFER){
            mb2_tag_framebuffer_t *fb = (mb2_tag_framebuffer_t*)tag;

            framebuffer = (volatile uint8_t*)(uintptr_t)fb->framebuffer_addr;
            pitch       = fb->framebuffer_pitch;
            g_width     = fb->framebuffer_width;
            g_height    = fb->framebuffer_height;
            g_bpp       = fb->framebuffer_bpp;
            fb_type     = fb->framebuffer_type;

            if(fb_type == 1){
                r_pos = fb->rgb.red_field_position;
                r_mask = fb->rgb.red_mask_size;
                g_pos = fb->rgb.green_field_position;
                g_mask = fb->rgb.green_mask_size;
                b_pos = fb->rgb.blue_field_position;
                b_mask = fb->rgb.blue_mask_size;
            } else if (fb_type == 0){
                serial_write("Indexed framebuffer not supported yet.\n");
            } else {
                serial_write("EGA text mode, not usable as LFB.\n");
            }

            serial_write("[FB]\n");
            log_u64(" addr: 0x", (uint64_t)(uintptr_t)framebuffer);
            log_u32(" pitch:", pitch);
            log_u32(" size :", g_width * g_height);
            log_u32(" wh   :", (g_width<<16) | g_height);
            log_u32(" bpp  :", g_bpp);
            log_u32(" type :", fb_type);
            log_u32(" rgb  :", (r_pos<<16)|(g_pos<<8)|b_pos);

            return; // done
        }
        addr = (addr + tag->size + 7) & ~((uintptr_t)7);
    }

    serial_write("No framebuffer tag found.\n");
}

static inline uint32_t scale_component(uint32_t v, uint8_t bits){
    if(bits == 8) return v & 0xFFu;
    if(bits == 5) return (v * 31u) / 255u;
    if(bits == 6) return (v * 63u) / 255u;
    return (bits>=8)? (v & 0xFFu) : (v >> (8-bits));
}

static inline uint32_t pack_rgb(uint32_t argb){
    // argb is 0xAARRGGBB
    uint32_t r = (argb >> 16) & 0xFF;
    uint32_t g = (argb >> 8)  & 0xFF;
    uint32_t b = (argb      ) & 0xFF;

    uint32_t R = scale_component(r, r_mask) << r_pos;
    uint32_t G = scale_component(g, g_mask) << g_pos;
    uint32_t B = scale_component(b, b_mask) << b_pos;

    // alpha ignored for LFB
    return R | G | B;
}

void draw_pixel(int x, int y, uint32_t argb){
    static uint32_t warned = 0;
    if(!framebuffer || x<0 || y<0 || (uint32_t)x>=g_width || (uint32_t)y>=g_height){
        if(!warned){ serial_write("draw_pixel: OOB or no FB.\n"); warned=1; }
        return;
    }
    uint32_t bpp_bytes = g_bpp >> 3;
    uint32_t off = (uint32_t)y * pitch + (uint32_t)x * bpp_bytes;
    volatile uint8_t *p = framebuffer + off;

    if (fb_type==1 && (g_bpp==32 || g_bpp==24)){
        uint32_t px = pack_rgb(argb);
        if(g_bpp==32){ *(volatile uint32_t*)p = px; }
        else { 
            // 24bpp little-endian
            p[0] = (uint8_t)(px >> 0);
            p[1] = (uint8_t)(px >> 8);
            p[2] = (uint8_t)(px >> 16);
        }
    } else {
        if(!warned){ serial_write("Unsupported FB format.\n"); warned=1; }
    }
}

void fill_screen(uint32_t argb){
    if(!framebuffer) return;
    for(uint32_t y=0; y<g_height; ++y){
        for(uint32_t x=0; x<g_width; ++x){
            draw_pixel((int)x,(int)y,argb);
        }
    }
}

void test_pattern(void){
    if(!framebuffer) return;
    // vertical RGB bars
    uint32_t w3 = g_width/3;
    for(uint32_t y=0; y<g_height; ++y){
        for(uint32_t x=0; x<g_width; ++x){
            uint32_t c = (x<w3)? 0xFFFF0000 : (x<2*w3? 0xFF00FF00 : 0xFF0000FF);
            draw_pixel((int)x,(int)y,c);
        }
    }
}

// Getters
uint32_t fb_width(void){ return g_width; }
uint32_t fb_height(void){ return g_height; }
uint32_t fb_bpp(void){ return g_bpp; }
uint64_t fb_addr(void){ return (uint64_t)(uintptr_t)framebuffer; }
uint32_t fb_pitch(void){ return pitch; }
