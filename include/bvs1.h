#ifndef _BVS1_H_
#define _BVS1_H_

#include <stdint.h>

#define bvs1_SCREEN_WIDTH 315
#define bvs1_SCREEN_HEIGHT 195

typedef enum {
	bvs1_MODE_GRAPHICS,
	bvs1_MODE_TEXT
} bvs1_framebuffer_mode;

typedef void (*bvs1_pixel_cb)(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b);

typedef struct {
	bvs1_framebuffer_mode bufMode;
	uint8_t *framebuffer;
	uint8_t *colorTable;
	uint8_t *paletteTable;
	bvs1_pixel_cb setPixel;
} bvs1_state;

void triad6_bvs1_init(bvs1_state *obj, bvs1_pixel_cb pixelCb);
void triad6_bvs1_render(bvs1_state *obj);
void triad6_bvs1_quit(bvs1_state *obj);

#endif
