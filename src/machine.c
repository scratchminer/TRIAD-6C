#include "machine.h"
#include "utility.h"

/*
	0200: P3 DC	SETBU #-1
	0202: 40 03	ADD #3
	0204: E0 02	CMP #2
	0206: D1 D9	RBPOS 0202
*/

void triad6_machine_init(machine_state *obj, cpu_read_cb readCb, cpu_write_cb writeCb) {
	triad6_cpu_init(&obj->cpu);
	triad6_bvs1_init(&obj->video);
	
	obj->ramSize = triad6_util_trytes2Bytes(triad6_util_kiloTrytes2Trytes(1));
	obj->RAM = malloc(obj->ramSize);
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
	// triad6_bvs1_render(&obj->video);
}

void triad6_machine_quit(machine_state *obj) {
	triad6_cpu_quit(&obj->cpu);
	triad6_bvs1_quit(&obj->video);
	free(obj->RAM);
}
