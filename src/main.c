#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL.h>
#include "bct.h"
#include "triad6.h"
#include "machine.h"
#include "utility.h"

static SDL_Window *window;
static SDL_Surface *surf;
static machine_state machine;

#define TRIAD6_FRAMEBUFFER_START (triad6_bct_uword_value(triad6_bct_uword_septemvigits("3000")))
#define TRIAD6_FRAMEBUFFER_END (triad6_bct_uword_value(triad6_bct_uword_septemvigits("6000")))
#define TRIAD6_COLOR_TABLE_START (triad6_bct_uword_value(triad6_bct_uword_septemvigits("B080")))
#define TRIAD6_PALETTE_TABLE_START (triad6_bct_uword_value(triad6_bct_uword_septemvigits("B090")))
#define TRIAD6_PALETTE_TABLE_END (triad6_bct_uword_value(triad6_bct_uword_septemvigits("B0A0")))

static bct_utryte readCb(bct_uword addr) {
	/*char buf[5];
	triad6_bct_uword_septemvigdump(addr, buf);
	buf[4] = '\0';
	printf("read %s\n", buf);
	*/
	uint32_t offset = triad6_bct_uword_value(addr);
	
	if (offset < machine.ramSize) {
		return triad6_bct_memory_read(machine.RAM, offset);
	}
	else if (offset < TRIAD6_FRAMEBUFFER_END) {
		return triad6_bct_memory_read(machine.video.framebuffer, offset - TRIAD6_FRAMEBUFFER_START);
	}
	else if (offset < TRIAD6_COLOR_TABLE_START) {
		return triad6_bct_utryte_convert(0);
	}
	else if (offset < TRIAD6_PALETTE_TABLE_START) {
		return triad6_bct_memory_read(machine.video.colorTable, offset - TRIAD6_COLOR_TABLE_START);
	}
	else if (offset < TRIAD6_PALETTE_TABLE_END) {
		return triad6_bct_memory_read(machine.video.paletteTable, offset - TRIAD6_PALETTE_TABLE_START);
	}
	else {
		return triad6_bct_utryte_convert(0);
	}
}

static void writeCb(bct_uword addr, bct_utryte data) {
	/*char addrBuf[5], dataBuf[3];
	triad6_bct_uword_septemvigdump(addr, addrBuf);
	triad6_bct_utryte_septemvigdump(data, dataBuf);
	addrBuf[4] = '\0';
	dataBuf[2] = '\0';
	printf("write %s -> %s\n", dataBuf, addrBuf);
	*/
	uint32_t offset = triad6_bct_uword_value(addr);
	
	if (offset < machine.ramSize) {
		triad6_bct_memory_write(machine.RAM, offset, data);
	}
	else if (offset < TRIAD6_FRAMEBUFFER_END) {
		triad6_bct_memory_write(machine.video.framebuffer, offset - TRIAD6_FRAMEBUFFER_START, data);
	}
	else if (offset < TRIAD6_COLOR_TABLE_START) {
		return;
	}
	else if ((offset >= TRIAD6_COLOR_TABLE_START) && (offset < TRIAD6_PALETTE_TABLE_START)) {
		triad6_bct_memory_write(machine.video.colorTable, offset - TRIAD6_COLOR_TABLE_START, data);
	}
	else if (offset < TRIAD6_PALETTE_TABLE_END) {
		triad6_bct_memory_write(machine.video.paletteTable, offset - TRIAD6_PALETTE_TABLE_START, data);
	}
}

static void pixelCb(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b) {
	SDL_PixelFormat *fmt = surf->format;
	uint32_t color = SDL_MapRGB(fmt, r, g, b);
	
	SDL_LockSurface(surf);
	uint32_t *pixels = (uint32_t *)surf->pixels;
	
	pixels[(2 * y) * (surf->pitch / fmt->BytesPerPixel) + (2 * x)] = color;
	pixels[(2 * y) * (surf->pitch / fmt->BytesPerPixel) + (2 * x + 1)] = color;
	pixels[(2 * y + 1) * (surf->pitch / fmt->BytesPerPixel) + (2 * x)] = color;
	pixels[(2 * y + 1) * (surf->pitch / fmt->BytesPerPixel) + (2 * x + 1)] = color;
	
	SDL_UnlockSurface(surf);
}

triad6_err triad6_init(void) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		return err_ERROR_INIT;
	}
	
	window = SDL_CreateWindow("TRIAD-6", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 630, 390, 0);
	if (window == NULL) {
		return err_ERROR_CREATE_WINDOW;
	}
	surf = SDL_GetWindowSurface(window);
	if (surf == NULL) {
		SDL_Quit();
		return err_ERROR_CREATE_WINDOW;
	}
	
	triad6_machine_init(&machine, readCb, writeCb, pixelCb);
	return err_OK;
}

void triad6_quit(void) {
	triad6_machine_quit(&machine);
	SDL_Quit();
}

triad6_err triad6_run(void) {
	bool stop = false;
	while (!stop)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					stop = true;
					break;
			}
		}
		
		triad6_machine_frame(&machine);
		SDL_UpdateWindowSurface(window);
	}
	
	return err_OK;
}

int main(int argc, char **argv) {
	triad6_err error = triad6_init();
	
	if (error == err_OK) {
		error = triad6_run();
	}
	triad6_quit();
	
	return -error;
}
