#include <string.h>

#include "bct.h"

bct_tryte triad6_bct_tryte_convert(int16_t value) {
	bct_tryte ret;
	
	if (value >= 0) {
		for (size_t tritOffset = 0; tritOffset < bct_TRYTE_SIZE; tritOffset++) {
			triad6_bct_setTrit(ret, tritOffset, (value + 1) % 3);
			value = (value + 1) / 3;
		}
	}
	else {
		for (size_t tritOffset = 0; tritOffset < bct_TRYTE_SIZE; tritOffset++) {
			triad6_bct_setTrit(ret, tritOffset, (value - 1) % 3);
			value = (value - 1) / 3;
		}
	}
	
	return triad6_bct_rollTryte(ret);
}

bct_tryte triad6_bct_tryte_tritfield(int16_t *trits, size_t size) {
	bct_tryte ret = triad6_bct_tryte_convert(0);
	
	for (size_t tritOffset = 1; (tritOffset <= size) && (tritOffset <= bct_TRYTE_SIZE); tritOffset++) {
		triad6_bct_setTrit(ret, tritOffset, trits[size - tritOffset]);
	}
	
	return triad6_bct_rollTryte(ret);
}

bct_tryte triad6_bct_tryte_septemvigits(char *septemvigits) {
	size_t size = strlen(septemvigits);
	bct_tryte ret = triad6_bct_tryte_convert(0);
	int16_t value;
	
	for (size_t offset = 0; (offset < size) && (offset < bct_TRYTE_SIZE / 3); offset++) {
		char septemvigit = septemvigits[offset];
		
		switch (septemvigit) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				value = (septemvigit - '0');
				break;
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
			case 'G':
			case 'H':
			case 'I':
			case 'J':
			case 'K':
			case 'L':
			case 'M':
			case 'N':
			case 'O':
			case 'P':
			case 'Q':
				value = (10 + (septemvigit - 'A'));
				break;
		}
		
		ret = triad6_bct_tryte_add(triad6_bct_tryte_shift_left(ret, 3), (bct_tryte)(triad6_bct_utryte_convert(value)));
	}
	
	return triad6_bct_rollTryte(ret);
}

int16_t triad6_bct_tryte_value(bct_tryte t) {
	int16_t ret = 0;
	int16_t power = 1;
	
	for (size_t tritOffset = 0; tritOffset < bct_TRYTE_SIZE; tritOffset++) {
		ret += (triad6_bct_getTrit(t, tritOffset) * power);
		power *= 3;
	}
	
	return ret;
}

void triad6_bct_tryte_septemvigdump(bct_tryte t, char *out) {
	for (size_t offset = 1; offset <= (bct_TRYTE_SIZE / 3); offset++) {
		char value = (char)(triad6_bct_tryte_value(triad6_bct_tryte_and(t, triad6_bct_tryte_septemvigits("Q"))));
		
		if (value < 10) {
			out[(bct_TRYTE_SIZE / 3) - offset] = '0' + value;
		}
		else {
			out[(bct_TRYTE_SIZE / 3) - offset] = 'A' + (value - 10);
		}
		
		t = triad6_bct_tryte_shift_right(t, 3);
	}
}

bct_tryte triad6_bct_tryte_add(bct_tryte t1, bct_tryte t2) {
	bct_tryte ret = 0;
	int16_t carryBorrow = 0;
	
	for (size_t tritOffset = 0; tritOffset < bct_TRYTE_SIZE; tritOffset++) {
		int16_t rawSum = triad6_bct_getTrit(t1, tritOffset) + triad6_bct_getTrit(t2, tritOffset) + carryBorrow;
		
		if (rawSum < bct_TRIT_MIN) {
			carryBorrow = -1;
			rawSum += 3;
		}
		else if (rawSum > bct_TRIT_MAX) {
			carryBorrow = 1;
			rawSum -= 3;
		}
		else {
			carryBorrow = 0;
		}
		
		triad6_bct_setTrit(ret, tritOffset, rawSum);
	}
	
	return ret;
}

bct_tryte triad6_bct_tryte_sub(bct_tryte t1, bct_tryte t2) {
	bct_utryte t2Comp = triad6_bct_tryte_inv(t2);
	return triad6_bct_tryte_add(t1, t2Comp);
}

bct_tryte triad6_bct_tryte_shift_left(bct_tryte t, int shiftAmount) {
	bct_tryte ret = t << (shiftAmount * bct_TRIT_SIZE);
	
	for (size_t tritOffset = 0; tritOffset < shiftAmount; tritOffset++) {
		triad6_bct_setTrit(ret, tritOffset, 0);
	}
	
	return triad6_bct_rollTryte(ret);
}

bct_tryte triad6_bct_tryte_shift_right(bct_tryte t, int shiftAmount) {
	bct_tryte ret = t >> (shiftAmount * bct_TRIT_SIZE);
	
	for (size_t tritOffset = bct_TRIT_SIZE * bct_TRYTE_SIZE; tritOffset > shiftAmount; tritOffset--) {
		triad6_bct_setTrit(ret, tritOffset, 0);
	}
	
	return triad6_bct_rollTryte(ret);
}

bct_tryte triad6_bct_tryte_inv(bct_tryte t) {
	bct_tryte ret = 0;
	int16_t tr;
	
	for (size_t tritOffset = 0; tritOffset < bct_TRYTE_SIZE; tritOffset++) {
		tr = triad6_bct_getTrit(t, tritOffset);
		triad6_bct_setTrit(ret, tritOffset, -tr);
	}
	
	return ret;
}

bct_tryte triad6_bct_tryte_or(bct_tryte t1, bct_tryte t2) {
	bct_tryte ret = 0;
	int16_t tr1, tr2;
	
	for (size_t tritOffset = 0; tritOffset < bct_TRYTE_SIZE; tritOffset++) {
		tr1 = triad6_bct_getTrit(t1, tritOffset);
		tr2 = triad6_bct_getTrit(t2, tritOffset);
		
		triad6_bct_setTrit(ret, tritOffset, (tr1 > tr2) ? tr1 : tr2);
	}
	
	return ret;
}

bct_tryte triad6_bct_tryte_and(bct_tryte t1, bct_tryte t2) {
	bct_tryte ret = 0;
	int16_t tr1, tr2;
	
	for (size_t tritOffset = 0; tritOffset < bct_TRYTE_SIZE; tritOffset++) {
		tr1 = triad6_bct_getTrit(t1, tritOffset);
		tr2 = triad6_bct_getTrit(t2, tritOffset);
		
		triad6_bct_setTrit(ret, tritOffset, (tr1 > tr2) ? tr2 : tr1);
	}
	
	return ret;
}

bct_tryte triad6_bct_tryte_mul(bct_tryte t1, bct_tryte t2) {
	bct_tryte ret = 0;
	int16_t tr1, tr2;
	
	for (size_t tritOffset = 0; tritOffset < bct_TRYTE_SIZE; tritOffset++) {
		tr1 = triad6_bct_getTrit(t1, tritOffset);
		tr2 = triad6_bct_getTrit(t2, tritOffset);
		
		triad6_bct_setTrit(ret, tritOffset, tr1 * tr2);
	}
	
	return ret;
}

bct_utryte triad6_bct_utryte_convert(uint16_t value) {
	bct_utryte ret = 0;
	
	for (size_t tritOffset = 0; tritOffset < bct_TRYTE_SIZE; tritOffset++) {
		triad6_bct_setUTrit(ret, tritOffset, value % 3);
		value /= 3;
	}
	
	return ret;
}

bct_utryte triad6_bct_utryte_septemvigits(char *septemvigits) {
	size_t size = strlen(septemvigits);
	bct_utryte ret = triad6_bct_utryte_convert(0);
	uint16_t value;
	
	for (size_t offset = 0; (offset < size) && (offset < bct_TRYTE_SIZE / 3); offset++) {
		char septemvigit = septemvigits[offset];
		
		switch (septemvigit) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				value = (septemvigit - '0');
				break;
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
			case 'G':
			case 'H':
			case 'I':
			case 'J':
			case 'K':
			case 'L':
			case 'M':
			case 'N':
			case 'O':
			case 'P':
			case 'Q':
				value = (10 + (septemvigit - 'A'));
				break;
		}
		
		ret = triad6_bct_utryte_add(triad6_bct_utryte_shift_left(ret, 3), triad6_bct_utryte_convert(value));
	}
	
	return triad6_bct_rollTryte(ret);
}

uint16_t triad6_bct_utryte_value(bct_utryte t) {
	uint16_t ret = 0;
	uint16_t power = 1;
	
	for (size_t tritOffset = 0; tritOffset < bct_TRYTE_SIZE; tritOffset++) {
		ret += triad6_bct_getUTrit(t, tritOffset) * power;
		power *= 3;
	}
	
	return ret;
}

void triad6_bct_utryte_septemvigdump(bct_utryte t, char *out) {
	for (size_t offset = 1; offset <= (bct_TRYTE_SIZE / 3); offset++) {
		char value = (char)(triad6_bct_utryte_value(triad6_bct_utryte_and(t, triad6_bct_utryte_septemvigits("Q"))));
		
		if (value < 10) {
			out[(bct_TRYTE_SIZE / 3) - offset] = '0' + value;
		}
		else {
			out[(bct_TRYTE_SIZE / 3) - offset] = 'A' + (value - 10);
		}
		
		t = triad6_bct_utryte_shift_right(t, 3);
	}
}

bct_utryte triad6_bct_utryte_add(bct_utryte t1, bct_utryte t2) {
	bct_utryte ret = 0;
	uint16_t carryBorrow = 0;
	
	for (size_t tritOffset = 0; tritOffset < bct_TRYTE_SIZE; tritOffset++) {
		uint16_t rawSum = triad6_bct_getUTrit(t1, tritOffset) + triad6_bct_getUTrit(t2, tritOffset) + carryBorrow;
		
		if (rawSum > bct_UTRIT_MAX) {
			carryBorrow = 1;
			rawSum -= 3;
		}
		else {
			carryBorrow = 0;
		}
		
		triad6_bct_setUTrit(ret, tritOffset, rawSum & ((1 << bct_TRIT_SIZE) - 1));
	}
	
	return ret;
}

bct_utryte triad6_bct_utryte_sub(bct_utryte t1, bct_utryte t2) {
	bct_utryte t2Comp = triad6_bct_utryte_add(triad6_bct_utryte_inv(t2), triad6_bct_utryte_convert(1));
	return triad6_bct_utryte_add(t1, t2Comp);
}

bct_utryte triad6_bct_utryte_shift_left(bct_utryte t, int shiftAmount) {
	bct_utryte ret = t << (shiftAmount * bct_TRIT_SIZE);
	return triad6_bct_rollTryte(ret);
}

bct_utryte triad6_bct_utryte_shift_right(bct_utryte t, int shiftAmount) {
	bct_utryte ret = t >> (shiftAmount * bct_TRIT_SIZE);
	return triad6_bct_rollTryte(ret);
}

bct_utryte triad6_bct_utryte_inv(bct_utryte t) {
	bct_utryte ret = 0;
	uint16_t tr;
	
	for (size_t tritOffset = 0; tritOffset < bct_TRYTE_SIZE; tritOffset++) {
		tr = triad6_bct_getUTrit(t, tritOffset);
		triad6_bct_setUTrit(ret, tritOffset, 2 - tr);
	}
	
	return ret;
}

bct_utryte triad6_bct_utryte_or(bct_utryte t1, bct_utryte t2) {
	bct_utryte ret = 0;
	uint16_t tr1, tr2;
	
	for (size_t tritOffset = 0; tritOffset < bct_TRYTE_SIZE; tritOffset++) {
		tr1 = triad6_bct_getUTrit(t1, tritOffset);
		tr2 = triad6_bct_getUTrit(t2, tritOffset);
		
		if (tr1 == 2 || tr2 == 2) {
			triad6_bct_setUTrit(ret, tritOffset, 2);
		}
		else if (tr1 == 1 && tr2 == 1) {
			triad6_bct_setUTrit(ret, tritOffset, 1);
		}
		else {
			triad6_bct_setUTrit(ret, tritOffset, 0);
		}
	}
	
	return ret;
}

bct_utryte triad6_bct_utryte_and(bct_utryte t1, bct_utryte t2) {
	bct_utryte ret = 0;
	uint16_t tr1, tr2;
	
	for (size_t tritOffset = 0; tritOffset < bct_TRYTE_SIZE; tritOffset++) {
		tr1 = triad6_bct_getUTrit(t1, tritOffset);
		tr2 = triad6_bct_getUTrit(t2, tritOffset);
		triad6_bct_setUTrit(ret, tritOffset, (tr1 > tr2) ? tr2 : tr1);
	}
	
	return ret;
}

bct_utryte triad6_bct_utryte_mul(bct_utryte t1, bct_utryte t2) {
	bct_utryte ret = 0;
	uint16_t tr1, tr2;
	
	for (size_t tritOffset = 0; tritOffset < bct_TRYTE_SIZE; tritOffset++) {
		tr1 = triad6_bct_getUTrit(t1, tritOffset);
		tr2 = triad6_bct_getUTrit(t2, tritOffset);
		
		triad6_bct_setUTrit(ret, tritOffset, tr1 * tr2);
	}
	
	return ret;
}

bct_uword triad6_bct_uword_convert(uint32_t value) {
	bct_uword ret = 0;
	
	for (size_t tritOffset = 0; tritOffset < bct_WORD_SIZE; tritOffset++) {
		triad6_bct_setUTrit(ret, tritOffset, value % 3);
		value /= 3;
	}
	
	return ret;
}

bct_uword triad6_bct_uword_septemvigits(char *septemvigits) {
	size_t size = strlen(septemvigits);
	bct_uword ret = triad6_bct_uword_convert(0);
	uint32_t value;
	
	for (size_t offset = 0; (offset < size) && (offset < bct_WORD_SIZE / 3); offset++) {
		char septemvigit = septemvigits[offset];
		
		switch (septemvigit) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				value = (septemvigit - '0');
				break;
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
			case 'G':
			case 'H':
			case 'I':
			case 'J':
			case 'K':
			case 'L':
			case 'M':
			case 'N':
			case 'O':
			case 'P':
			case 'Q':
				value = (10 + (septemvigit - 'A'));
				break;
		}
		
		ret = triad6_bct_uword_add(triad6_bct_uword_shift_left(ret, 3), triad6_bct_uword_convert(value));
	}
	
	return triad6_bct_rollWord(ret);
}

uint32_t triad6_bct_uword_value(bct_uword t) {
	uint32_t ret = 0;
	uint32_t power = 1;
	
	for (size_t tritOffset = 0; tritOffset < bct_WORD_SIZE; tritOffset++) {
		ret += triad6_bct_getUTrit(t, tritOffset) * power;
		power *= 3;
	}
	
	return ret;
}

void triad6_bct_uword_septemvigdump(bct_uword t, char *out) {
	for (size_t offset = 1; offset <= (bct_WORD_SIZE / 3); offset++) {
		char value = (char)(triad6_bct_uword_value(triad6_bct_uword_and(t, triad6_bct_uword_septemvigits("Q"))));
		
		if (value < 10) {
			out[(bct_WORD_SIZE / 3) - offset] = '0' + value;
		}
		else {
			out[(bct_WORD_SIZE / 3) - offset] = 'A' + (value - 10);
		}
		
		t = triad6_bct_uword_shift_right(t, 3);
	}
}

bct_uword triad6_bct_uword_sub(bct_uword t1, bct_uword t2) {
	bct_uword t2Comp = triad6_bct_uword_add(triad6_bct_uword_inv(t2), triad6_bct_uword_convert(1));
	return triad6_bct_uword_add(t1, t2Comp);
}

bct_uword triad6_bct_uword_add(bct_uword t1, bct_uword t2) {
	bct_uword ret = 0;
	uint32_t carryBorrow = 0;
	
	for (size_t tritOffset = 0; tritOffset < bct_WORD_SIZE; tritOffset++) {
		uint16_t rawSum = triad6_bct_getUTrit(t1, tritOffset) + triad6_bct_getUTrit(t2, tritOffset) + carryBorrow;
		
		if (rawSum > bct_UTRIT_MAX) {
			carryBorrow = 1;
			rawSum -= 3;
		}
		else {
			carryBorrow = 0;
		}
		
		triad6_bct_setUTrit(ret, tritOffset, rawSum & ((1 << bct_TRIT_SIZE) - 1));
	}
	
	return triad6_bct_rollWord(ret);
}

bct_uword triad6_bct_uword_shift_left(bct_uword t, int shiftAmount) {
	bct_uword ret = t << (shiftAmount * bct_TRIT_SIZE);
	return triad6_bct_rollWord(ret);
}

bct_uword triad6_bct_uword_shift_right(bct_uword t, int shiftAmount) {
	bct_uword ret = t >> (shiftAmount * bct_TRIT_SIZE);
	return triad6_bct_rollWord(ret);
}

bct_uword triad6_bct_uword_inv(bct_uword t) {
	bct_uword ret = 0;
	uint32_t tr;
	
	for (size_t tritOffset = 0; tritOffset < bct_WORD_SIZE; tritOffset++) {
		tr = triad6_bct_getUTrit(t, tritOffset);
		triad6_bct_setUTrit(ret, tritOffset, 2 - tr);
	}
	
	return ret;
}

bct_uword triad6_bct_uword_or(bct_uword t1, bct_uword t2) {
	bct_uword ret = 0;
	uint32_t tr1, tr2;
	
	for (size_t tritOffset = 0; tritOffset < bct_WORD_SIZE; tritOffset++) {
		tr1 = triad6_bct_getUTrit(t1, tritOffset);
		tr2 = triad6_bct_getUTrit(t2, tritOffset);
		
		if (tr1 == 2 || tr2 == 2) {
			triad6_bct_setUTrit(ret, tritOffset, 2);
		}
		else if (tr1 == 1 && tr2 == 1) {
			triad6_bct_setUTrit(ret, tritOffset, 1);
		}
		else {
			triad6_bct_setUTrit(ret, tritOffset, 0);
		}
	}
	
	return ret;
}

bct_uword triad6_bct_uword_and(bct_uword t1, bct_uword t2) {
	bct_uword ret = 0;
	uint32_t tr1, tr2;
	
	for (size_t tritOffset = 0; tritOffset < bct_WORD_SIZE; tritOffset++) {
		tr1 = triad6_bct_getUTrit(t1, tritOffset);
		tr2 = triad6_bct_getUTrit(t2, tritOffset);
		triad6_bct_setUTrit(ret, tritOffset, (tr1 > tr2) ? tr2 : tr1);
	}
	
	return ret;
}

bct_uword triad6_bct_uword_mul(bct_uword t1, bct_uword t2) {
	bct_uword ret = 0;
	uint32_t tr1, tr2;
	
	for (size_t tritOffset = 0; tritOffset < bct_WORD_SIZE; tritOffset++) {
		tr1 = triad6_bct_getUTrit(t1, tritOffset);
		tr2 = triad6_bct_getUTrit(t2, tritOffset);
		
		triad6_bct_setUTrit(ret, tritOffset, tr1 * tr2);
	}
	
	return ret;
}

bct_utryte triad6_bct_memory_read(const uint8_t *src, size_t offset) {
	bct_utryte ret;
	
	size_t bitOffset = offset * bct_TRIT_SIZE * bct_TRYTE_SIZE;
	size_t byteOffset = bitOffset / 8;
	bitOffset = bitOffset % 8;
	
	const uint16_t *addr = (const uint16_t *)&src[byteOffset];
	if (bitOffset) {
		ret = triad6_bct_rollTryte((bct_utryte)(*addr >> (16 - (bct_TRIT_SIZE * bct_TRYTE_SIZE))));
	}
	else {
		ret = triad6_bct_rollTryte((bct_utryte)(*addr));
	}
	
	return ret;
}

void triad6_bct_memory_write(const uint8_t *dst, size_t offset, bct_tryte data) {
	size_t bitOffset = offset * bct_TRIT_SIZE * bct_TRYTE_SIZE;
	size_t byteOffset = bitOffset / 8;
	bitOffset = bitOffset % 8;
	
	uint16_t *addr = (uint16_t *)&dst[byteOffset];
	if (bitOffset) {
		*addr |= triad6_bct_rollTryte(data) << (16 - (bct_TRIT_SIZE * bct_TRYTE_SIZE));
	}
	else {
		*addr |= triad6_bct_rollTryte(data);
	}
}