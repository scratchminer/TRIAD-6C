#ifndef _BCT_H_
#define _BCT_H_

#include <stdint.h>
#include <stdlib.h>

// in bits
#define bct_TRIT_SIZE 2

// in trits
#define bct_TRYTE_SIZE 6
#define bct_WORD_SIZE (bct_TRYTE_SIZE * 2)

// min and max values
#define bct_TRIT_MIN -1
#define bct_TRIT_MAX 1
#define bct_UTRIT_MIN 0
#define bct_UTRIT_MAX 2
#define bct_TRYTE_MIN -364
#define bct_TRYTE_MAX 364
#define bct_UTRYTE_MIN 0
#define bct_UTRYTE_MAX 728
#define bct_UWORD_MIN 0
#define bct_UWORD_MAX 531441

#define triad6_bct_getTrit(t, trit) ((((t) >> ((trit) * bct_TRIT_SIZE)) & ((1 << bct_TRIT_SIZE) - 1)) - 2)
#define triad6_bct_setTrit(t, trit, val) ((t) |= ((((val) + 2) & ((1 << bct_TRIT_SIZE) - 1)) << ((trit) * bct_TRIT_SIZE)))
#define triad6_bct_getUTrit(t, trit) (((t) >> ((trit) * bct_TRIT_SIZE)) & ((1 << bct_TRIT_SIZE) - 1))
#define triad6_bct_setUTrit(t, trit, val) ((t) |= (((val) & ((1 << bct_TRIT_SIZE) - 1)) << ((trit) * bct_TRIT_SIZE)))

#define triad6_bct_rollTryte(t) ((t) & ((1 << (bct_TRIT_SIZE * bct_TRYTE_SIZE)) - 1))
#define triad6_bct_rollWord(t) ((t) & ((1 << (bct_TRIT_SIZE * bct_WORD_SIZE)) - 1))

#define triad6_bct_tryte2utryte(t) (triad6_bct_utryte_convert((int16_t)triad6_bct_tryte_value(t)))
#define triad6_bct_tryte2uword(t) (triad6_bct_uword_convert((uint32_t)triad6_bct_tryte_value(t)))
#define triad6_bct_utryte2tryte(t) (triad6_bct_tryte_convert(triad6_bct_utryte_value(t)))
#define triad6_bct_utryte2uword(t) ((bct_uword)(t))
#define triad6_bct_uword2tryte(t) (triad6_bct_tryte_convert((uint16_t)triad6_bct_uword_value(triad6_bct_rollTryte(t))))
#define triad6_bct_uword2utryte(t) (triad6_bct_rollTryte(t))

typedef int16_t bct_tryte;
typedef uint16_t bct_utryte;
typedef uint32_t bct_uword;

// convert: convert a binary value to a BCT value
// value: convert a BCT value to a binary value

// add: add two BCT values
// shift_left: shift a BCT value left
// shift_right: shift a BCT value right

// inv: invert a BCT value
// or: tritwise OR of two BCT values
// and: tritwise AND of two BCT values
// mul: tritwise multiplication of two BCT values (or XOR)

// get a bct_tryte from an array of trit values
bct_tryte triad6_bct_tryte_tritfield(int16_t *trits, size_t size);

// get a bct_tryte from a septemvigesimal string
bct_tryte triad6_bct_tryte_septemvigits(char *septemvigits);

bct_tryte triad6_bct_tryte_convert(int16_t value);
int16_t triad6_bct_tryte_value(bct_tryte t);

bct_tryte triad6_bct_tryte_add(bct_tryte t1, bct_tryte t2);
bct_tryte triad6_bct_tryte_shift_left(bct_tryte t, int shiftAmount);
bct_tryte triad6_bct_tryte_shift_right(bct_tryte t, int shiftAmount);

bct_tryte triad6_bct_tryte_inv(bct_tryte t);

/* equivalent to a bitwise OR
*  matrix:
*  - 0 +
*  0 0 +
*  + + +
*/
bct_tryte triad6_bct_tryte_or(bct_tryte t1, bct_tryte t2);

/* equivalent to a bitwise AND
*  matrix:
*  - - -
*  - 0 0
*  - 0 +
*/
bct_tryte triad6_bct_tryte_and(bct_tryte t1, bct_tryte t2);

/* equivalent to a bitwise XOR
*  matrix:
*  + 0 -
*  0 0 0
*  - 0 +
*/
bct_tryte triad6_bct_tryte_mul(bct_tryte t1, bct_tryte t2);


bct_utryte triad6_bct_utryte_convert(uint16_t value);
bct_utryte triad6_bct_utryte_septemvigits(char *septemvigits);
uint16_t triad6_bct_utryte_value(bct_utryte t);

bct_utryte triad6_bct_utryte_add(bct_utryte t1, bct_utryte t2);
bct_utryte triad6_bct_utryte_shift_left(bct_utryte t, int shiftAmount);
bct_utryte triad6_bct_utryte_shift_right(bct_utryte t, int shiftAmount);

bct_utryte triad6_bct_utryte_inv(bct_utryte t);

/* equivalent to a bitwise OR
*  matrix:
*  0 0 2
*  0 1 2
*  2 2 2
*/
bct_utryte triad6_bct_utryte_or(bct_utryte t1, bct_utryte t2);

/* equivalent to a bitwise AND
*  matrix:
*  0 1 0
*  1 1 1
*  0 1 2
*/
bct_utryte triad6_bct_utryte_and(bct_utryte t1, bct_utryte t2);

/* equivalent to a bitwise XOR
*  matrix:
*  0 0 0
*  0 1 2
*  0 2 1
*/
bct_utryte triad6_bct_utryte_mul(bct_utryte t1, bct_utryte t2);


bct_uword triad6_bct_uword_convert(uint32_t value);
bct_uword triad6_bct_uword_septemvigits(char *septemvigits);
uint32_t triad6_bct_uword_value(bct_uword t);

bct_uword triad6_bct_uword_add(bct_uword t1, bct_uword t2);
bct_uword triad6_bct_uword_sub(bct_uword t1, bct_uword t2);
bct_uword triad6_bct_uword_shift_left(bct_uword t, int shiftAmount);
bct_uword triad6_bct_uword_shift_right(bct_uword t, int shiftAmount);

bct_uword triad6_bct_uword_inv(bct_uword t);
bct_uword triad6_bct_uword_or(bct_uword t1, bct_uword t2);
bct_uword triad6_bct_uword_and(bct_uword t1, bct_uword t2);
bct_uword triad6_bct_uword_mul(bct_uword t1, bct_uword t2);

// helper functions to pack a BCT array as tightly as possible
bct_utryte triad6_bct_memory_read(const uint8_t *src, size_t offset);
void triad6_bct_memory_write(const uint8_t *dst, size_t offset, bct_tryte data);

#endif
