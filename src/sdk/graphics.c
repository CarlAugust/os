#include <stdint.h>
#include <kernel/framebuffer.h>
#include <sdk/graphics.h>

frame_buffer_info fb_info;

void init_window() {
	while (set_frame_buffer_info(&fb_info) < 0);
}

inline uint8_t *fb_back() {

	// Default to use zero because of a problem with QEMU???
	/*
		TODO: Is there some way to fix framebuffering not working???	
	*/
    return fb_info.buf[0];
}

static inline void fill_pixel(uint32_t offset, rgb color) {
	uint8_t *p = fb_back();
	p[offset] = color.red;
	p[offset + 1] = color.green;
	p[offset + 2] = color.blue;
	p[offset + 3] = 0xFF;
}

void draw_rectangle(uint32_t x, uint32_t y, uint32_t w, uint32_t h, rgb color) {

    uint32_t row = fb_info.pitch * y;
    for (uint32_t cy = 0; cy < h && cy + y < fb_info.height; cy++) {
        uint32_t px = row + x * (fb_info.px_size);

		for (uint32_t cx = 0; cx < w && cx + x < fb_info.width; cx++) {
            fill_pixel(px, color);
            px += fb_info.px_size;
		}

        row += fb_info.pitch;
    }
}

void draw_circle(uint32_t x, uint32_t y, uint32_t r, rgb color) {
	// Move to top left
	uint32_t y1 = y - r;
	y1 > fb_info.height ? y1 = 0 : 0;
	uint32_t x1 = x - r;
	x1 > fb_info.width ? x1 = 0 : 0;
	uint32_t r2 = r*r;

	uint32_t row = fb_info.pitch * y1;

    for (uint32_t cy = 0; cy < 2 * r && cy + y1 < fb_info.height; cy++) {

        uint32_t px = row + x * (fb_info.px_size);

		uint32_t d2 = (y - (cy + y1));
		uint32_t d2sqr = d2 * d2;

		for (uint32_t cx = 0; cx < 2 * r && cx + x1 < fb_info.width; cx++) {
			uint32_t d1 = (x - (cx + x1));
			uint32_t d1sqr = d1 * d1;

			if (d1sqr + d2sqr <= r2) {
				fill_pixel(px, color);
			}

            px += fb_info.px_size;
		}

        row += fb_info.pitch;
    }

}

void clear_background(rgb color) {
    draw_rectangle(0, 0, fb_info.width, fb_info.height, color);
}

uint32_t screen_width() {
    return fb_info.width;
}

uint32_t screen_height() {
    return fb_info.height;
}