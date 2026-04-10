#include "graphics.h"
#include "kernel.h"
#include "helper.h"

#define HEIGHT 480
#define WIDTH 640
#define DEPTH 32
#define MBOX_FB_CH 1

enum {
	TAG_SET_PHYSICAL = 0x00048003,
	TAG_SET_VIRTUAL = 0x00048004,
	TAG_SET_DEPTH = 0x00048005,
	TAG_SET_V_OFFSET = 0x00048009,
	TAG_ALLOC_BUFFER = 0x00040001,
	TAG_GET_PITCH = 0x00040008,
};

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
	uint32_t x_offset;
	uint32_t y_offset;

    uint32_t pitch;
    uint32_t px_size;

	uint8_t *buf[2];
    int active;

} frame_buffer_info;


static volatile uint32_t __attribute__((aligned(16))) mbox[36];
frame_buffer_info fb_info; 
int init_frame_buffer() {
	// 1 << 22 is just some free space lmao
	uart_puts("INIT_FRAME_BUFFER: _________________________________\r\n");


    mbox[0] = 35*4;
    mbox[1] = 0;

    mbox[2] = 0x48003;  //set phy wh
    mbox[3] = 8;
    mbox[4] = 8;
    mbox[5] = WIDTH;         //FrameBufferInfo.width
    mbox[6] = HEIGHT;          //FrameBufferInfo.height

    mbox[7] = 0x48004;  //set virt wh
    mbox[8] = 8;
    mbox[9] = 8;
    mbox[10] = WIDTH;        //FrameBufferInfo.virtual_width
    mbox[11] = HEIGHT * 2;         //FrameBufferInfo.virtual_height

    mbox[12] = 0x48009; //set virt offset
    mbox[13] = 8;
    mbox[14] = 0;
    mbox[15] = 0;           //FrameBufferInfo.x_offset
    mbox[16] = 0;           //FrameBufferInfo.y.offset

    mbox[17] = 0x48005; //set depth
    mbox[18] = 4;
    mbox[19] = 4;
    mbox[20] = 32;          //FrameBufferInfo.depth

    mbox[21] = 0x48006; //set pixel order
    mbox[22] = 4;
    mbox[23] = 4;
    mbox[24] = 1;           //RGB, not BGR preferably

    mbox[25] = 0x40001; //get framebuffer, gets alignment on request
    mbox[26] = 8;
    mbox[27] = 8;
    mbox[28] = 4096;        //FrameBufferInfo.pointer
    mbox[29] = 0;           //FrameBufferInfo.size

    mbox[30] = 0x40008; //get pitch
    mbox[31] = 4;
    mbox[32] = 4;
    mbox[33] = 0;           //FrameBufferInfo.pitch

    mbox[34] = 0;

	mbox_write((uint32_t)mbox, 8);
	mbox_read(8);

	if (mbox[1] != 0x80000000) {
		printf("Error: setting up frame_buffer\r\n");
		return -1;
	};


	fb_info.height = HEIGHT;
	fb_info.width = WIDTH;

	fb_info.x_offset = 0;
	fb_info.y_offset = 0;

    fb_info.px_size = DEPTH / 8;
    fb_info.pitch = mbox[33];

    uint32_t addr  = mbox[28] & 0x3FFFFFFF; // strip GPU bus address bit

    fb_info.active = 0;

    fb_info.buf[0] = (uint8_t *)addr;
    fb_info.buf[1] = (uint8_t *)(addr + fb_info.pitch * fb_info.height);

	memset((void*)mbox, 0, sizeof(mbox));

	uart_puts("INIT_FRAME_BUFFER: ___________________________\r\n");

	return 0;
}

int frame_buffer_swap() {
    int next = 1 - fb_info.active;

	mbox[0]  = 8 * 4;
	mbox[1]  = 0x00000000;
	mbox[2]  = 0x00048009;  // set virtual offset
	mbox[3]  = 8;
	mbox[4]  = 0;
	mbox[5]  = 0;
	mbox[6]  = next * fb_info.height;

	mbox[7] = 0x00000000;  // end tag

    mbox_write((uint32_t)mbox, 8);
    mbox_read(8);

	if (mbox[1] != 0x80000000) {
		printf("Error: swapping frame_buffer\r\n");
		return -1;
	};

    fb_info.active = next;

    memset((void*)mbox, 0, sizeof(mbox));  

	return 0;
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