#pragma once

#include <stdint.h>

typedef struct frame_buffer_info {
	uint32_t width;
	uint32_t height;
	uint32_t x_offset;
	uint32_t y_offset;

    uint32_t pitch;
	
	uint32_t px_depth;
    uint32_t px_size;

	uint8_t *buf[2];
    int active;

} frame_buffer_info;

int mbox_set_frame_buffer_info(frame_buffer_info *fb_info);