#include "graphics.h"
#include "kernel.h"

#define MBOX_FB_CH 1

typedef struct frame_buffer_msg {
	uint32_t width;
	uint32_t height;
	uint32_t virtual_width;
	uint32_t virtual_height;

	// Set by GPU
	uint32_t pitch;

	// Amount of bits per pixel, 24 for RGB
	uint32_t depth;

	// Should be 0
	uint32_t x_offset;
	uint32_t y_offset;

	// Set by GPU
	uint32_t pointer;

	// Set by GPU
	uint32_t size;
} frame_buffer_msg;

typedef struct frame_buffer_info {
	uint32_t width;
	uint32_t height;

	uint32_t c_width;
	uint32_t c_height;

	uint32_t x_offset;
	uint32_t y_offset;

	// Set by GPU
	volatile uint8_t* pointer;
	// Set by GPU
	uint32_t size;

} frame_buffer_info;

frame_buffer_info fb_info; 
int init_frame_buffer() {
	// 1 << 22 is just some free space lmao
	uart_puts("INIT_FRAME_BUFFER: Starting initilization of frame buffer\r\n");
	volatile struct frame_buffer_msg *fb = (volatile struct frame_buffer_msg *)(1 << 22);

	fb->width = 640;
	fb->height = 480;
	fb->virtual_width = fb->width;
	fb->virtual_height = fb->height;

	fb->pitch = 0;
	fb->depth = 24;
	fb->pointer = 0;
	fb->size = 0;

	fb->x_offset = 0;
	fb->y_offset = 0;

	uart_puts("INIT_FRAME_BUFFER: Writing to mbox\r\n");
	mbox_write((uint32_t)fb, MBOX_FB_CH);

	uart_puts("INIT_FRAME_BUFFER: Reading from mbox\r\n");
	uint32_t r = mbox_read(MBOX_FB_CH);

	if (r) {
		uart_puts("ERROR: Writing to MBOC\r\n");
		return -1;
	}

	if (!fb->pointer) {
		uart_puts("ERROR: MBOX didnt return pointer\r\n");
		return -1;
	}


	fb_info.height = fb->height;
	fb_info.width = fb->width;
	fb_info.c_height = fb_info.height / sizeof(uint8_t);
	fb_info.c_width = fb_info.width / sizeof(uint8_t);

	fb_info.x_offset = 0;
	fb_info.y_offset = 1;

	fb_info.pointer = (volatile uint8_t*)(fb->pointer);
	fb_info.size = fb->size;

	uart_puts("INIT_FRAME_BUFFER: initilization success\r\n");

	return 0;
}

void screen_set(rgb color) {

	for (uint32_t cy = 0; cy < fb_info.c_height; cy++) {
		for (uint32_t cx = 0; cx < fb_info.c_width; cx++) {
			uint32_t offset = ((cy * fb_info.c_width) + cx) * 3;

			fb_info.pointer[offset] = color.red;
			fb_info.pointer[offset + 1] = color.green;
			fb_info.pointer[offset + 2] = color.blue;
		}
	}
}