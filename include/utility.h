#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#if defined(__linux)
#include <time.h>
#ifdef CLOCK_MONOTONIC
#define CLOCKID CLOCK_MONOTONIC
#else
#define CLOCKID CLOCK_REALTIME
#endif
#elif defined(__APPLE__)
#include <mach/mach_time.h>
#endif

#include "bct.h"

#define PERF_COUNTER_UNITS 1000000000

/*
*  from https://www.roxlu.com/2014/047/high-resolution-timer-function-in-c-c--/
*  Get a performance counter in nanoseconds
*/

static uint64_t triad6_util_perfCounter(void) {
	static uint64_t is_init = 0;
#if defined(__APPLE__)
	static mach_timebase_info_data_t info;
	if (0 == is_init) {
		mach_timebase_info(&info);
		is_init = 1;
	}
	uint64_t now;
	now = mach_absolute_time();
	now *= info.numer;
	now /= info.denom;
	return now;
#elif defined(__linux)
	static struct timespec linux_rate;
	if (0 == is_init) {
		clock_getres(CLOCKID, &linux_rate);
		is_init = 1;
	}
	uint64_t now;
	struct timespec spec;
	clock_gettime(CLOCKID, &spec);
	now = spec.tv_sec * 1.0e9 + spec.tv_nsec;
	return now;
#endif
}

/*
*  Kilotrytes -> trytes conversion
*/
static inline size_t triad6_util_kiloTrytes2Trytes(size_t kilotrytes) {
	size_t result = kilotrytes;
	for (size_t i = 0; i < 10; ++i) {
		result *= 3;
	}
	return result;
}

/*
*  Trytes -> BCT bytes conversion
*/
static inline size_t triad6_util_trytes2Bytes(size_t trytes)
{
	size_t result = trytes * bct_TRIT_SIZE * bct_TRYTE_SIZE;
	size_t tmp = result / 8;
	result = tmp + ((result % 8) ? 1 : 0);
	return result;
}

static char *triad6_util_septemvigdump(uint8_t *data, size_t size) {
	// reserve space for the null terminator
	size_t bufSize = 1;
	
	for (size_t addr = 0; addr < size; addr += 9) {
		bufSize += strlen("0000: ");
		for (size_t offset = 0; offset < 9; offset++) {
			bufSize += strlen("00 ");
		}
	}
	
	char *ret = calloc(bufSize, sizeof(char));
	char *ptr = ret;
	
	int n;
	char tryteEnd;
	
	for (size_t addr = 0; addr < size; addr += 9) {
		strncpy(ptr, "0000: ", strlen("0000: "));
		triad6_bct_uword_septemvigdump(triad6_bct_uword_convert(addr), ptr);
		ptr += strlen("0000: ");
		
		for (size_t offset = 0; offset < 9; offset++) {
			if (offset == 8) {
				tryteEnd = '\n';
			}
			else {
				tryteEnd = ' ';
			}
			
			strncpy(ptr, "00", strlen("00"));
			ptr[2] = tryteEnd;
			triad6_bct_utryte_septemvigdump(triad6_bct_memory_read(data, addr + offset), ptr);
			ptr += strlen("00 ");
		}
	}
	
	return ret;
}

#endif