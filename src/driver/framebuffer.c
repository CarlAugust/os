#include <driver/framebuffer.h>
#include <kernel/base.h>
#include <kernel/printf.h>

#define HEIGHT 480
#define WIDTH 640

// DEPTH 32 is RGBA
#define DEPTH 32


enum {
	TAG_SET_PHYSICAL = 0x00048003,
	TAG_SET_VIRTUAL = 0x00048004,
	TAG_SET_DEPTH = 0x00048005,
	TAG_SET_V_OFFSET = 0x00048009,
	TAG_ALLOC_BUFFER = 0x00040001,
	TAG_GET_PITCH = 0x00040008,
};


static inline uint32_t mbox_read(uint32_t channel) {
	uint32_t r = 0;
	do {
		// Loop untill mbox data is ready
		while (mmio_read(MBOX_STATUS) & (1U << 30));
		r = mmio_read(MBOX_READ);
	} while ((r & 0xF) != channel);

	return r & 0xFFFFFFF0;
}

static inline void mbox_write(uint32_t v, uint32_t channel) {
	// Loop untill mbox isnt full
	while (mmio_read(MBOX_STATUS) & (1U << 31));
	mmio_write(MBOX_WRITE, channel | (v & 0xFFFFFFF0));

}

static volatile uint32_t __attribute__((aligned(16))) mbox[36];
int mbox_set_frame_buffer_info(frame_buffer_info *fb_info) {
    
	printf("INIT_FRAME_BUFFER: _________________________________\n");


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
		printf("Error: setting up frame_buffer\n");
		return -1;
	};


	fb_info->height = HEIGHT;
	fb_info->width = WIDTH;

	fb_info->x_offset = 0;
	fb_info->y_offset = 0;

    fb_info->px_size = DEPTH / 8;
    fb_info->pitch = mbox[33];

    uint32_t addr  = mbox[28] & 0x3FFFFFFF; // strip GPU bus address bit

    fb_info->active = 0;

    fb_info->buf[0] = (uint8_t *)addr;
    fb_info->buf[1] = (uint8_t *)(addr + fb_info->pitch * fb_info->height);

	printf("INIT_FRAME_BUFFER: ___________________________\n");

	return 0;
}

/*

Just here for documentation when i want to add dual frame buffering or whatever its called
Since it doesnt seem to work on QEMU

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
*/