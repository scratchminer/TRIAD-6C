#include "machine.h"
#include "utility.h"

/*
	Bandock's test:
	
	0200: P3 DC	SETBU #-1
	0202: 40 03	ADD #3
	0204: E0 02	CMP #2
	0206: D1 D9	RBPOS 0202
*/

/*
	Color change test (assumes the screen is cleared to color 0):
	(WARNING: DO NOT RUN AT FULL SPEED)
	
	0200: P3 DC	SETBU #-1
	0202: 20 00	LDA #0
	0204: 30 90 B0	STA B090
	0207: 40 01	ADD #1
	0209: D0 D8	RJMP 0204
*/

void triad6_machine_init(machine_state *obj, cpu_read_cb readCb, cpu_write_cb writeCb, bvs1_pixel_cb pixelCb) {
	triad6_cpu_init(&obj->cpu);
	triad6_bvs1_init(&obj->video, pixelCb);
	
	obj->ramSize = triad6_util_kiloTrytes2Trytes(1);
	obj->RAM = malloc(triad6_util_trytes2Bytes(obj->ramSize));
	obj->cpu.instPtr = triad6_bct_uword_septemvigits("0200");
	
	#define ROM(off, val) writeCb(triad6_bct_uword_add(obj->cpu.instPtr, triad6_bct_uword_convert(off)), triad6_bct_utryte_septemvigits(val))
	ROM(0, "P3");
	ROM(1, "DC");
	ROM(2, "40");
	ROM(3, "03");
	ROM(4, "E0");
	ROM(5, "02");
	ROM(6, "D1");
	ROM(7, "D9");
	#undef ROM
	
	/*
	char *dump = triad6_util_septemvigdump(obj->RAM, obj->ramSize);
	printf("%s", dump);
	free(dump);
	*/
	
	obj->cpu.clockPeriod = PERF_COUNTER_UNITS / machine_CPU_FREQUENCY;
	obj->cpu.readTryte = readCb;
	obj->cpu.writeTryte = writeCb;
}

void triad6_machine_frame(machine_state *obj) {
	triad6_cpu_execute(&obj->cpu);
	triad6_bvs1_render(&obj->video);
}

void triad6_machine_quit(machine_state *obj) {
	triad6_cpu_quit(&obj->cpu);
	triad6_bvs1_quit(&obj->video);
	free(obj->RAM);
}
