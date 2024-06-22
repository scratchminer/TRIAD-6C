#include <stdlib.h>

#include "bct.h"
#include "bvs1.h"
#include "colors.h"
#include "utility.h"

void triad6_bvs1_init(bvs1_state *obj, bvs1_pixel_cb pixelCb) {
	obj->bufMode = bvs1_MODE_GRAPHICS;
	obj->framebuffer = malloc(triad6_util_trytes2Bytes(triad6_util_kiloTrytes2Trytes(1)));
	obj->colorTable = malloc(triad6_util_trytes2Bytes(27));
	obj->paletteTable = malloc(triad6_util_trytes2Bytes(27));
	obj->setPixel = pixelCb;
}

void triad6_bvs1_render(bvs1_state *obj) {
	bct_utryte pixel;
	uint16_t paletteIndex;
	uint16_t colorIndex;
	uint16_t r, g, b;
	
	for (uint16_t y = 0; y < bvs1_SCREEN_HEIGHT; y++) {
		for (uint16_t x = 0; x < bvs1_SCREEN_WIDTH; x++) {
			if (x % 2 == 0) {
				// left pixel is the 3 most significant trits
				pixel = triad6_bct_memory_read(obj->framebuffer, (y * bvs1_SCREEN_WIDTH + x) / 2);
				paletteIndex = triad6_bct_utryte_value(pixel) / 27;
			}
			else {
				// right pixel is the 3 least significant trits
				paletteIndex = triad6_bct_utryte_value(pixel) % 27;
			}
			
			// color table lookup
			colorIndex = triad6_bct_utryte_value(triad6_bct_memory_read(obj->colorTable, paletteIndex)) % 27;
			
			// color tryte is RRGGBB
			uint16_t color = triad6_bct_utryte_value(triad6_bct_memory_read(obj->paletteTable, colorIndex));
			r = bvs1_triad6_colors[3 * color];
			g = bvs1_triad6_colors[3 * color + 1];
			b = bvs1_triad6_colors[3 * color + 2];
			
			obj->setPixel(x, y, (uint8_t)r, (uint8_t)g, (uint8_t)b);
		}
	}
}

void triad6_bvs1_quit(bvs1_state *obj) {
	free(obj->framebuffer);
	free(obj->colorTable);
	free(obj->paletteTable);
}