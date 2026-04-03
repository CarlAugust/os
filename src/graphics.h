#pragma once

#include <stdint.h>

typedef struct rgb {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
} rgb;

int init_frame_buffer();
void screen_set(rgb color);
