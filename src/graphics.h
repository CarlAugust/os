#pragma once

#include <stdint.h>

typedef struct rgb {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
} rgb;

int init_frame_buffer();
void clear_background(rgb color);
void draw_rectangle(uint32_t x, uint32_t y, uint32_t w, uint32_t h, rgb color);
