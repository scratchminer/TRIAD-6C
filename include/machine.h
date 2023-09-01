#ifndef _MACHINE_H_
#define _MACHINE_H_

#include <stdint.h>

#include "bct.h"
#include "cpu.h"
#include "bvs1.h"

#define machine_CPU_FREQUENCY 1000000

typedef struct {
	cpu_state cpu;
	bvs1_state video;
	uint8_t *RAM;
	size_t ramSize;
} machine_state;

void triad6_machine_init(machine_state *obj, cpu_read_cb readCb, cpu_write_cb writeCb);
void triad6_machine_frame(machine_state *obj);
void triad6_machine_quit(machine_state *obj);

#endif
