#ifndef _CPU_H_
#define _CPU_H_

#include <stdbool.h>
#include <stdint.h>

#include "bct.h"

typedef enum {
	cpu_FLAG_ZERO = 0,
	cpu_FLAG_OVERFLOW,
	cpu_FLAG_CARRY,
	cpu_FLAG_INTERRUPT,
	cpu_FLAG_BALANCED
} triad6_flag;

typedef bct_utryte (*cpu_read_cb)(bct_uword addr);
typedef void (*cpu_write_cb)(bct_uword addr, bct_utryte data);

typedef struct {
	bct_tryte A;
	bct_tryte C;
	bct_tryte D;
	bct_tryte X;
	bct_tryte Y;
	bct_tryte F;
	bct_uword PC;
	bct_uword SP;
	
	uint64_t startClockTime;
	uint64_t lastClockTime;
	uint64_t clockPeriod;
	
	bct_uword mar;
	bct_tryte mdr;
	bct_tryte ir;
	bct_uword instPtr;
	
	uint8_t thisCycle;
	bool fetch;
	
	bct_uword tmp;
	
	cpu_read_cb readTryte;
	cpu_write_cb writeTryte;
} cpu_state;

typedef void (*cpu_instruction_cb)(cpu_state *obj);

void triad6_cpu_init(cpu_state *obj);
void triad6_cpu_execute(cpu_state *obj);
void triad6_cpu_quit(cpu_state *obj);

#endif
