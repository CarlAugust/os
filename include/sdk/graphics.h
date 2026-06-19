#pragma once

#include <stdint.h>

typedef struct rgba {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;
} rgba;

#define RED (rgba){255, 0, 0}

void init_window();

void clear_background(rgba color);
void draw_rectangle(uint32_t x, uint32_t y, uint32_t w, uint32_t h, rgba color);
void draw_circle(uint32_t x, uint32_t y, uint32_t r, rgba color);
