#include <stdlib.h>

#include "bvs1.h"
#include "utility.h"

void triad6_bvs1_init(bvs1_state *obj) {
	obj->bufMode = bvs1_MODE_GRAPHICS;
	obj->framebuffer = malloc(triad6_util_trytes2Bytes(triad6_util_kiloTrytes2Trytes(1)));
	obj->colorTable = malloc(triad6_util_trytes2Bytes(27));
	obj->palettes = malloc(triad6_util_trytes2Bytes(27));
}

void triad6_bvs1_quit(bvs1_state *obj) {
	free(obj->framebuffer);
	free(obj->colorTable);
	free(obj->palettes);
}