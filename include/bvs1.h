#ifndef _BVS1_H_
#define _BVS1_H_

#include <stdint.h>

typedef enum {
	bvs1_MODE_GRAPHICS,
	bvs1_MODE_TEXT
} bvs1_framebuffer_mode;

typedef struct {
	bvs1_framebuffer_mode bufMode;
	uint8_t *framebuffer;
	uint8_t *colorTable;
	uint8_t *palettes;
} bvs1_state;

void triad6_bvs1_init(bvs1_state *obj);
void triad6_bvs1_quit(bvs1_state *obj);

#endif
