#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL.h>
#include "bct.h"
#include "triad6.h"
#include "machine.h"

static SDL_Window *window;
static machine_state machine;

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
	else if (offset < triad6_bct_uword_septemvigits("6000")) {
		return triad6_bct_memory_read(machine.video.framebuffer, offset);
	}
	else if (offset < triad6_bct_uword_septemvigits("B080")) {
		return triad6_bct_utryte_convert(0);
	}
	else if (offset < triad6_bct_uword_septemvigits("B090")) {
		return triad6_bct_memory_read(machine.video.colorTable, offset);
	}
	else if (offset < triad6_bct_uword_septemvigits("B0A0")) {
		return triad6_bct_memory_read(machine.video.palettes, offset);
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
	else if (offset < triad6_bct_uword_septemvigits("6000")) {
		triad6_bct_memory_write(machine.video.framebuffer, offset, data);
	}
}

triad6_err triad6_init(void) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		return err_ERROR_INIT;
	}
	
	window = SDL_CreateWindow("TRIAD-6", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 630, 390, 0);
	if (window == NULL) {
		return err_ERROR_CREATE_WINDOW;
	}
	else {
		triad6_machine_init(&machine, readCb, writeCb);
		return err_OK;
	}
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

int main(int argc, char *argv[]) {
	triad6_err error = triad6_init();
	
	if (error == err_OK) {
		error = triad6_run();
	}
	triad6_quit();
	
	return -error;
}
