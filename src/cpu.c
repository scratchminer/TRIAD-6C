#include <stdbool.h>
#include <stdlib.h>
#include <setjmp.h>

#include "cpu.h"
#include "utility.h"

static jmp_buf renderEnv;

#define FRAMERATE 60

#define CONCAT_(one, two, three, four) one##two##three##four

#define cpu_INSTDEF(name) CONCAT_(static void cpu_inst_,name,,)(cpu_state *obj)
#define cpu_GET_INST(name) CONCAT_(cpu_inst_,name,,)
#define cpu_EXEC_INST(name) cpu_GET_INST(name)(obj)
#define cpu_WAIT_CLOCK(obj) longjmp(renderEnv, 1)

#define cpu_INDEX_CONVERT(obj, index) ((triad6_bct_getTrit(obj->F, cpu_FLAG_BALANCED) == 1) ? triad6_bct_tryte2uword(index) : triad6_bct_utryte2uword(index))
#define cpu_INCR_UWORD(reg) reg = triad6_bct_uword_add((reg), triad6_bct_uword_convert(1))

cpu_INSTDEF(invalid) {
	// do nothing
}

// No OPeration
cpu_INSTDEF(NOP) {
	cpu_WAIT_CLOCK(obj);
}

// Read a tryte from the address pointed to by PC
cpu_INSTDEF(rdpc) {
	obj->mar = obj->instPtr;
	obj->mdr = obj->readTryte(obj->mar);
	cpu_INCR_UWORD(obj->instPtr);
	cpu_WAIT_CLOCK(obj);
}

// Read a tryte from the address pointed to by MAR
cpu_INSTDEF(rdmar) {
	obj->mdr = obj->readTryte(obj->mar);
	cpu_WAIT_CLOCK(obj);
}

// Read a tryte from the address pointed to by MAR, then increment MAR
cpu_INSTDEF(rdmarinc) {
	obj->mdr = obj->readTryte(obj->mar);
	cpu_INCR_UWORD(obj->mar);
	cpu_WAIT_CLOCK(obj);
}

// Write a tryte to the address pointed to by MAR
cpu_INSTDEF(wrmar) {
	obj->writeTryte(obj->mar, obj->mdr);
	cpu_WAIT_CLOCK(obj);
}

#define cpu_ADDRESS_IMM(obj) do { \
	cpu_EXEC_INST(rdpc); \
} while (0)

#define cpu_ADDRESS_ABS(obj) do { \
	bct_uword tmp; \
	cpu_EXEC_INST(rdpc); \
	tmp = triad6_bct_utryte2uword((obj)->mdr); \
	cpu_EXEC_INST(rdpc); \
	(obj)->mar = triad6_bct_uword_or(tmp, triad6_bct_uword_shift_left((obj)->mdr, bct_TRYTE_SIZE)); \
	cpu_EXEC_INST(rdmar); \
} while (0)

#define cpu_ADDRESS_ABX(obj) do { \
	bct_uword tmp; \
	cpu_EXEC_INST(rdpc); \
	tmp = triad6_bct_utryte2uword((obj)->mdr); \
	cpu_EXEC_INST(rdpc); \
	(obj)->mar = triad6_bct_uword_add(triad6_bct_uword_or(tmp, triad6_bct_uword_shift_left((obj)->mdr, bct_TRYTE_SIZE)), cpu_INDEX_CONVERT(obj, (obj)->X)); \
	cpu_EXEC_INST(rdmar); \
} while (0)

#define cpu_ADDRESS_ABY(obj) do { \
	bct_uword tmp; \
	cpu_EXEC_INST(rdpc); \
	tmp = triad6_bct_utryte2uword((obj)->mdr); \
	cpu_EXEC_INST(rdpc); \
	(obj)->mar = triad6_bct_uword_add(triad6_bct_uword_or(tmp, triad6_bct_uword_shift_left((obj)->mdr, bct_TRYTE_SIZE)), cpu_INDEX_CONVERT(obj, (obj)->Y)); \
	cpu_EXEC_INST(rdmar); \
} while (0)

#define cpu_ADDRESS_IND(obj) do { \
	bct_uword tmp; \
	cpu_EXEC_INST(rdpc); \
	tmp = triad6_bct_utryte2uword((obj)->mdr); \
	cpu_EXEC_INST(rdpc); \
	(obj)->mar = triad6_bct_uword_or(tmp, triad6_bct_uword_shift_left((obj)->mdr, bct_TRYTE_SIZE)); \
	cpu_EXEC_INST(rdmarinc); \
	tmp = triad6_bct_utryte2uword((obj)->mdr); \
	cpu_EXEC_INST(rdmar); \
	(obj)->mar = triad6_bct_uword_or(tmp, triad6_bct_uword_shift_left((obj)->mdr, bct_TRYTE_SIZE)); \
	cpu_EXEC_INST(rdmar); \
} while (0)

#define cpu_ADDRESS_IDX(obj) do { \
	bct_uword tmp; \
	cpu_EXEC_INST(rdpc); \
	tmp = triad6_bct_utryte2uword((obj)->mdr); \
	cpu_EXEC_INST(rdpc); \
	(obj)->mar = triad6_bct_uword_or(tmp, triad6_bct_uword_shift_left((obj)->mdr, bct_TRYTE_SIZE)); \
	cpu_EXEC_INST(rdmarinc); \
	tmp = triad6_bct_utryte2uword((obj)->mdr); \
	cpu_EXEC_INST(rdmar); \
	(obj)->mar = triad6_bct_uword_add(triad6_bct_uword_or(tmp, triad6_bct_uword_shift_left((obj)->mdr, bct_TRYTE_SIZE)), cpu_INDEX_CONVERT(obj, (obj)->X)); \
	cpu_EXEC_INST(rdmar); \
} while (0)

#define cpu_ADDRESS_IDY(obj) do { \
	bct_uword tmp; \
	cpu_EXEC_INST(rdpc); \
	tmp = triad6_bct_utryte2uword((obj)->mdr); \
	cpu_EXEC_INST(rdpc); \
	(obj)->mar = triad6_bct_uword_or(tmp, triad6_bct_uword_shift_left((obj)->mdr, bct_TRYTE_SIZE)); \
	cpu_EXEC_INST(rdmarinc); \
	tmp = triad6_bct_utryte2uword((obj)->mdr); \
	cpu_EXEC_INST(rdmar); \
	(obj)->mar = triad6_bct_uword_add(triad6_bct_uword_or(tmp, triad6_bct_uword_shift_left((obj)->mdr, bct_TRYTE_SIZE)), cpu_INDEX_CONVERT(obj, (obj)->Y)); \
	cpu_EXEC_INST(rdmar); \
} while (0)

#define cpu_SIGN(n) ((n) > 0 ? 1 : ((n) < 0 ? -1 : 0))
#define cpu_LD_INSTDEF(reg, mode) cpu_INSTDEF(CONCAT_(LD,reg,_,mode)) { \
	CONCAT_(cpu_ADDRESS_,mode,,)(obj); \
	obj->reg = obj->mdr; \
	if (triad6_bct_getTrit(obj->F, cpu_FLAG_BALANCED) == 1) { \
		triad6_bct_setTrit(obj->F, cpu_FLAG_ZERO, cpu_SIGN(triad6_bct_tryte_value(obj->reg))); \
	} \
	else if (triad6_bct_getTrit(obj->F, cpu_FLAG_BALANCED) == -1) { \
		triad6_bct_setTrit(obj->F, cpu_FLAG_ZERO, cpu_SIGN(triad6_bct_utryte_value(obj->reg))); \
	} \
}

#define cpu_ST_INSTDEF(reg, mode) cpu_INSTDEF(CONCAT_(ST,reg,_,mode)) { \
	CONCAT_(cpu_ADDRESS_,mode,,)(obj); \
	obj->mdr = obj->reg; \
	cpu_EXEC_INST(wrmar); \
}

#define cpu_ADD_INSTDEF(mode) cpu_INSTDEF(CONCAT_(ADD_,mode,,)) { \
	CONCAT_(cpu_ADDRESS_,mode,,)(obj); \
	if (triad6_bct_getTrit(obj->F, cpu_FLAG_BALANCED) == 1) { \
		bct_tryte tmp = triad6_bct_tryte_add(obj->A, obj->mdr); \
		if (triad6_bct_getTrit(tmp, bct_TRYTE_SIZE) != 0) { \
			triad6_bct_setTrit(obj->F, cpu_FLAG_CARRY, triad6_bct_getTrit(tmp, bct_TRYTE_SIZE)); \
		} \
		if (triad6_bct_tryte_value(tmp) < triad6_bct_tryte_value(obj->A)) { \
			triad6_bct_setTrit(obj->F, cpu_FLAG_OVERFLOW, 1); \
		} \
		else { \
			triad6_bct_setTrit(obj->F, cpu_FLAG_OVERFLOW, 0); \
		} \
		triad6_bct_setTrit(obj->F, cpu_FLAG_ZERO, cpu_SIGN(triad6_bct_tryte_value(tmp))); \
		obj->A = tmp; \
	} \
	else if (triad6_bct_getTrit(obj->F, cpu_FLAG_BALANCED) == -1) { \
		bct_utryte tmp = triad6_bct_utryte_add(obj->A, obj->mdr); \
		if (triad6_bct_getUTrit(tmp, bct_TRYTE_SIZE) != 0) { \
			triad6_bct_setUTrit(obj->F, cpu_FLAG_CARRY, triad6_bct_getUTrit(tmp, bct_TRYTE_SIZE)); \
		} \
		if (triad6_bct_utryte_value(tmp) < triad6_bct_utryte_value(obj->A)) { \
			triad6_bct_setTrit(obj->F, cpu_FLAG_OVERFLOW, 1); \
		} \
		triad6_bct_setTrit(obj->F, cpu_FLAG_ZERO, cpu_SIGN(triad6_bct_utryte_value(tmp))); \
		obj->A = tmp; \
	} \
}

#define cpu_ADDC_INSTDEF(mode) cpu_INSTDEF(CONCAT_(ADDC_,mode,,)) { \
	CONCAT_(cpu_ADDRESS_,mode,,)(obj); \
	if (triad6_bct_getTrit(obj->F, cpu_FLAG_BALANCED) == 1) { \
		bct_tryte tmp = triad6_bct_tryte_add(triad6_bct_tryte_add(obj->A, obj->mdr), triad6_bct_getUTrit(obj->F, cpu_FLAG_CARRY)); \
		if (triad6_bct_getTrit(tmp, bct_TRYTE_SIZE) != 0) { \
			triad6_bct_setTrit(obj->F, cpu_FLAG_CARRY, triad6_bct_getTrit(tmp, bct_TRYTE_SIZE)); \
		} \
		if (triad6_bct_tryte_value(tmp) < triad6_bct_tryte_value(obj->A)) { \
			triad6_bct_setTrit(obj->F, cpu_FLAG_OVERFLOW, 1); \
		} \
		else { \
			triad6_bct_setTrit(obj->F, cpu_FLAG_OVERFLOW, 0); \
		} \
		triad6_bct_setTrit(obj->F, cpu_FLAG_ZERO, cpu_SIGN(triad6_bct_tryte_value(tmp))); \
		obj->A = tmp; \
	} \
	else if (triad6_bct_getTrit(obj->F, cpu_FLAG_BALANCED) == -1) { \
		bct_utryte tmp = triad6_bct_utryte_add(triad6_bct_utryte_add(obj->A, obj->mdr), triad6_bct_getUTrit(obj->F, cpu_FLAG_CARRY)); \
		if (triad6_bct_getUTrit(tmp, bct_TRYTE_SIZE) != 0) { \
			triad6_bct_setUTrit(obj->F, cpu_FLAG_CARRY, triad6_bct_getUTrit(tmp, bct_TRYTE_SIZE)); \
		} \
		if (triad6_bct_utryte_value(tmp) < triad6_bct_utryte_value(obj->A)) { \
			triad6_bct_setTrit(obj->F, cpu_FLAG_OVERFLOW, 1); \
		} \
		triad6_bct_setTrit(obj->F, cpu_FLAG_ZERO, cpu_SIGN(triad6_bct_utryte_value(tmp))); \
		obj->A = tmp; \
	} \
}

#define cpu_ADDB_INSTDEF(mode) cpu_INSTDEF(CONCAT_(ADDB_,mode,,)) { \
	CONCAT_(cpu_ADDRESS_,mode,,)(obj); \
	if (triad6_bct_getTrit(obj->F, cpu_FLAG_BALANCED) == 1) { \
		bct_tryte tmp = triad6_bct_tryte_add(triad6_bct_tryte_add(obj->A, obj->mdr), bct_UTRIT_MAX - triad6_bct_getUTrit(obj->F, cpu_FLAG_CARRY)); \
		if (triad6_bct_getTrit(tmp, bct_TRYTE_SIZE) != 0) { \
			triad6_bct_setTrit(obj->F, cpu_FLAG_CARRY, triad6_bct_getTrit(tmp, bct_TRYTE_SIZE)); \
		} \
		if (triad6_bct_tryte_value(tmp) < triad6_bct_tryte_value(obj->A)) { \
			triad6_bct_setTrit(obj->F, cpu_FLAG_OVERFLOW, 1); \
		} \
		else { \
			triad6_bct_setTrit(obj->F, cpu_FLAG_OVERFLOW, 0); \
		} \
		triad6_bct_setTrit(obj->F, cpu_FLAG_ZERO, cpu_SIGN(triad6_bct_tryte_value(tmp))); \
		obj->A = tmp; \
	} \
	else if (triad6_bct_getTrit(obj->F, cpu_FLAG_BALANCED) == -1) { \
		bct_utryte tmp = triad6_bct_utryte_add(triad6_bct_utryte_add(obj->A, obj->mdr), bct_UTRIT_MAX - triad6_bct_getUTrit(obj->F, cpu_FLAG_CARRY)); \
		if (triad6_bct_getUTrit(tmp, bct_TRYTE_SIZE) != 0) { \
			triad6_bct_setUTrit(obj->F, cpu_FLAG_CARRY, triad6_bct_getUTrit(tmp, bct_TRYTE_SIZE)); \
		} \
		if (triad6_bct_utryte_value(tmp) < triad6_bct_utryte_value(obj->A)) { \
			triad6_bct_setTrit(obj->F, cpu_FLAG_OVERFLOW, 1); \
		} \
		triad6_bct_setTrit(obj->F, cpu_FLAG_ZERO, cpu_SIGN(triad6_bct_utryte_value(tmp))); \
		obj->A = tmp; \
	} \
}

#define cpu_SUB_INSTDEF(mode) cpu_INSTDEF(CONCAT_(SUB_,mode,,)) { \
	CONCAT_(cpu_ADDRESS_,mode,,)(obj); \
	if (triad6_bct_getTrit(obj->F, cpu_FLAG_BALANCED) == 1) { \
		bct_tryte tmp = triad6_bct_tryte_add(obj->A, obj->mdr); \
		if (triad6_bct_getTrit(tmp, bct_TRYTE_SIZE) != 0) { \
			triad6_bct_setTrit(obj->F, cpu_FLAG_CARRY, triad6_bct_getTrit(tmp, bct_TRYTE_SIZE)); \
		} \
		if (triad6_bct_tryte_value(tmp) > triad6_bct_tryte_value(obj->A)) { \
			triad6_bct_setTrit(obj->F, cpu_FLAG_OVERFLOW, -1); \
		} \
		else { \
			triad6_bct_setTrit(obj->F, cpu_FLAG_OVERFLOW, 0); \
		} \
		triad6_bct_setTrit(obj->F, cpu_FLAG_ZERO, cpu_SIGN(triad6_bct_tryte_value(tmp))); \
		obj->A = tmp; \
	} \
	else if (triad6_bct_getTrit(obj->F, cpu_FLAG_BALANCED) == -1) { \
		bct_utryte tmp = triad6_bct_utryte_add(obj->A, triad6_bct_rollTryte(triad6_bct_utryte_add(triad6_bct_utryte_inv(obj->mdr), triad6_bct_utryte_convert(1)))); \
		if (triad6_bct_getUTrit(tmp, bct_TRYTE_SIZE) != 0) { \
			triad6_bct_setUTrit(obj->F, cpu_FLAG_CARRY, triad6_bct_getUTrit(tmp, bct_TRYTE_SIZE)); \
		} \
		if (triad6_bct_utryte_value(tmp) > triad6_bct_utryte_value(obj->A)) { \
			triad6_bct_setTrit(obj->F, cpu_FLAG_OVERFLOW, -1); \
		} \
		triad6_bct_setTrit(obj->F, cpu_FLAG_ZERO, cpu_SIGN(triad6_bct_utryte_value(tmp))); \
		obj->A = tmp; \
	} \
}

#define cpu_XFER_TRYTE_INSTDEF(dst, src) cpu_INSTDEF(CONCAT_(T,src,dst,)) { \
	obj->dst = obj->src; \
	cpu_WAIT_CLOCK(obj); \
}

#define cpu_BRANCH_INSTDEF(opName, condExpr) cpu_INSTDEF(opName) { \
	bct_uword tmp; \
	cpu_EXEC_INST(rdpc); \
	tmp = triad6_bct_uword_or(triad6_bct_uword_and(obj->instPtr, triad6_bct_uword_inv(triad6_bct_utryte2uword(bct_UTRYTE_MAX))), triad6_bct_utryte2uword(obj->mdr)); \
	cpu_EXEC_INST(rdpc); \
	tmp = triad6_bct_uword_or(triad6_bct_uword_shift_left(triad6_bct_utryte2uword(obj->mdr), bct_TRYTE_SIZE), tmp); \
	if (condExpr) { \
		obj->instPtr = tmp; \
		cpu_WAIT_CLOCK(obj); \
	} \
}

#define cpu_RBRANCH_INSTDEF(opName, condExpr) cpu_INSTDEF(CONCAT_(R,opName,,)) { \
	cpu_EXEC_INST(rdpc); \
	if (condExpr) { \
		obj->instPtr = triad6_bct_uword_sub(obj->PC, cpu_INDEX_CONVERT(obj, obj->mdr)); \
		cpu_WAIT_CLOCK(obj); \
	} \
}

#define cpu_SET_FLAG_INSTDEF(flagName, flagEnum) cpu_INSTDEF(CONCAT_(SET,flagName,,)) { \
	cpu_EXEC_INST(rdpc); \
	triad6_bct_setTrit(obj->F, CONCAT_(cpu_FLAG_,flagEnum,,), triad6_bct_getTrit(obj->mdr, 0)); \
}

cpu_LD_INSTDEF(A, IMM);
cpu_LD_INSTDEF(C, IMM);
cpu_LD_INSTDEF(D, IMM);
cpu_LD_INSTDEF(X, IMM);
cpu_LD_INSTDEF(Y, IMM);

cpu_LD_INSTDEF(A, ABS);
cpu_LD_INSTDEF(C, ABS);
cpu_LD_INSTDEF(D, ABS);
cpu_LD_INSTDEF(X, ABS);
cpu_LD_INSTDEF(Y, ABS);

cpu_LD_INSTDEF(A, ABX);
cpu_LD_INSTDEF(C, ABX);
cpu_LD_INSTDEF(D, ABX);
cpu_LD_INSTDEF(Y, ABX);

cpu_LD_INSTDEF(A, ABY);
cpu_LD_INSTDEF(C, ABY);
cpu_LD_INSTDEF(D, ABY);
cpu_LD_INSTDEF(X, ABY);

cpu_LD_INSTDEF(A, IND);
cpu_LD_INSTDEF(D, IND);

cpu_LD_INSTDEF(A, IDX);
cpu_LD_INSTDEF(D, IDX);

cpu_LD_INSTDEF(A, IDY);
cpu_LD_INSTDEF(D, IDY);

cpu_ST_INSTDEF(A, ABS);
cpu_ST_INSTDEF(C, ABS);
cpu_ST_INSTDEF(D, ABS);
cpu_ST_INSTDEF(X, ABS);
cpu_ST_INSTDEF(Y, ABS);

cpu_ST_INSTDEF(A, ABX);
cpu_ST_INSTDEF(C, ABX);
cpu_ST_INSTDEF(D, ABX);
cpu_ST_INSTDEF(Y, ABX);

cpu_ST_INSTDEF(A, ABY);
cpu_ST_INSTDEF(C, ABY);
cpu_ST_INSTDEF(D, ABY);
cpu_ST_INSTDEF(X, ABY);

cpu_ADD_INSTDEF(IMM);
cpu_ADD_INSTDEF(ABS);
cpu_ADD_INSTDEF(ABX);
cpu_ADD_INSTDEF(ABY);
cpu_ADD_INSTDEF(IND);
cpu_ADD_INSTDEF(IDX);
cpu_ADD_INSTDEF(IDY);

cpu_INSTDEF(ADDD) {
	if (triad6_bct_getTrit(obj->F, cpu_FLAG_BALANCED) == 1) {
		bct_tryte tmp = triad6_bct_tryte_add(obj->A, obj->D);
		if (triad6_bct_getTrit(tmp, bct_TRYTE_SIZE) != 0) {
			triad6_bct_setTrit(obj->F, cpu_FLAG_CARRY, triad6_bct_getTrit(tmp, bct_TRYTE_SIZE));
		}
		if (triad6_bct_tryte_value(tmp) < triad6_bct_tryte_value(obj->A)) {
			triad6_bct_setTrit(obj->F, cpu_FLAG_OVERFLOW, 1);
		}
		else {
			triad6_bct_setTrit(obj->F, cpu_FLAG_OVERFLOW, 0);
		}
		triad6_bct_setTrit(obj->F, cpu_FLAG_ZERO, cpu_SIGN(triad6_bct_tryte_value(tmp)));
		obj->A = tmp;
	}
	else if (triad6_bct_getTrit(obj->F, cpu_FLAG_BALANCED) == -1) {
		bct_utryte tmp = triad6_bct_utryte_add(obj->A, obj->D);
		if (triad6_bct_getUTrit(tmp, bct_TRYTE_SIZE) != 0) {
			triad6_bct_setUTrit(obj->F, cpu_FLAG_CARRY, triad6_bct_getUTrit(tmp, bct_TRYTE_SIZE));
		}
		if (triad6_bct_utryte_value(tmp) < triad6_bct_utryte_value(obj->A)) {
			triad6_bct_setTrit(obj->F, cpu_FLAG_OVERFLOW, 1);
		}
		triad6_bct_setTrit(obj->F, cpu_FLAG_ZERO, cpu_SIGN(triad6_bct_utryte_value(tmp)));
		obj->A = tmp;
	}
}

cpu_ADDC_INSTDEF(IMM);
cpu_ADDC_INSTDEF(ABS);
cpu_ADDC_INSTDEF(ABX);
cpu_ADDC_INSTDEF(ABY);
cpu_ADDC_INSTDEF(IND);
cpu_ADDC_INSTDEF(IDX);
cpu_ADDC_INSTDEF(IDY);

cpu_INSTDEF(ADDDC) {
	if (triad6_bct_getTrit(obj->F, cpu_FLAG_BALANCED) == 1) {
		bct_tryte tmp = triad6_bct_tryte_add(triad6_bct_tryte_add(obj->A, obj->D), triad6_bct_getUTrit(obj->F, cpu_FLAG_CARRY));
		if (triad6_bct_getTrit(tmp, bct_TRYTE_SIZE) != 0) {
			triad6_bct_setTrit(obj->F, cpu_FLAG_CARRY, triad6_bct_getTrit(tmp, bct_TRYTE_SIZE));
		}
		if (triad6_bct_tryte_value(tmp) < triad6_bct_tryte_value(obj->A)) {
			triad6_bct_setTrit(obj->F, cpu_FLAG_OVERFLOW, 1);
		}
		else {
			triad6_bct_setTrit(obj->F, cpu_FLAG_OVERFLOW, 0);
		}
		triad6_bct_setTrit(obj->F, cpu_FLAG_ZERO, cpu_SIGN(triad6_bct_tryte_value(tmp)));
		obj->A = tmp;
	}
	else if (triad6_bct_getTrit(obj->F, cpu_FLAG_BALANCED) == -1) {
		bct_utryte tmp = triad6_bct_utryte_add(triad6_bct_utryte_add(obj->A, obj->D), triad6_bct_getUTrit(obj->F, cpu_FLAG_CARRY));
		if (triad6_bct_getUTrit(tmp, bct_TRYTE_SIZE) != 0) {
			triad6_bct_setUTrit(obj->F, cpu_FLAG_CARRY, triad6_bct_getUTrit(tmp, bct_TRYTE_SIZE));
		}
		if (triad6_bct_utryte_value(tmp) < triad6_bct_utryte_value(obj->A)) {
			triad6_bct_setTrit(obj->F, cpu_FLAG_OVERFLOW, 1);
		}
		triad6_bct_setTrit(obj->F, cpu_FLAG_ZERO, cpu_SIGN(triad6_bct_utryte_value(tmp)));
		obj->A = tmp;
	}
}

cpu_ADDB_INSTDEF(IMM);
cpu_ADDB_INSTDEF(ABS);
cpu_ADDB_INSTDEF(ABX);
cpu_ADDB_INSTDEF(ABY);
cpu_ADDB_INSTDEF(IND);
cpu_ADDB_INSTDEF(IDX);
cpu_ADDB_INSTDEF(IDY);

cpu_INSTDEF(ADDDB) {
	if (triad6_bct_getTrit(obj->F, cpu_FLAG_BALANCED) == 1) {
		bct_tryte tmp = triad6_bct_tryte_add(triad6_bct_tryte_add(obj->A, obj->D), bct_UTRIT_MAX - triad6_bct_getUTrit(obj->F, cpu_FLAG_CARRY));
		if (triad6_bct_getTrit(tmp, bct_TRYTE_SIZE) != 0) {
			triad6_bct_setTrit(obj->F, cpu_FLAG_CARRY, triad6_bct_getTrit(tmp, bct_TRYTE_SIZE));
		}
		if (triad6_bct_tryte_value(tmp) < triad6_bct_tryte_value(obj->A)) {
			triad6_bct_setTrit(obj->F, cpu_FLAG_OVERFLOW, 1);
		}
		else {
			triad6_bct_setTrit(obj->F, cpu_FLAG_OVERFLOW, 0);
		}
		triad6_bct_setTrit(obj->F, cpu_FLAG_ZERO, cpu_SIGN(triad6_bct_tryte_value(tmp)));
		obj->A = tmp;
	}
	else if (triad6_bct_getTrit(obj->F, cpu_FLAG_BALANCED) == -1) {
		bct_utryte tmp = triad6_bct_utryte_add(triad6_bct_utryte_add(obj->A, obj->D), bct_UTRIT_MAX - triad6_bct_getUTrit(obj->F, cpu_FLAG_CARRY));
		if (triad6_bct_getUTrit(tmp, bct_TRYTE_SIZE) != 0) {
			triad6_bct_setUTrit(obj->F, cpu_FLAG_CARRY, triad6_bct_getUTrit(tmp, bct_TRYTE_SIZE));
		}
		if (triad6_bct_utryte_value(tmp) < triad6_bct_utryte_value(obj->A)) {
			triad6_bct_setTrit(obj->F, cpu_FLAG_OVERFLOW, 1);
		}
		triad6_bct_setTrit(obj->F, cpu_FLAG_ZERO, cpu_SIGN(triad6_bct_utryte_value(tmp)));
		obj->A = tmp;
	}
}

cpu_SUB_INSTDEF(IMM);

cpu_XFER_TRYTE_INSTDEF(A, C);
cpu_XFER_TRYTE_INSTDEF(A, D);
cpu_XFER_TRYTE_INSTDEF(A, X);
cpu_XFER_TRYTE_INSTDEF(A, Y);

cpu_XFER_TRYTE_INSTDEF(C, A);
cpu_XFER_TRYTE_INSTDEF(C, D);
cpu_XFER_TRYTE_INSTDEF(C, X);
cpu_XFER_TRYTE_INSTDEF(C, Y);

cpu_XFER_TRYTE_INSTDEF(D, A);
cpu_XFER_TRYTE_INSTDEF(D, C);
cpu_XFER_TRYTE_INSTDEF(D, X);
cpu_XFER_TRYTE_INSTDEF(D, Y);

cpu_XFER_TRYTE_INSTDEF(X, A);
cpu_XFER_TRYTE_INSTDEF(X, C);
cpu_XFER_TRYTE_INSTDEF(X, D);
cpu_XFER_TRYTE_INSTDEF(X, Y);

cpu_XFER_TRYTE_INSTDEF(Y, A);
cpu_XFER_TRYTE_INSTDEF(Y, C);
cpu_XFER_TRYTE_INSTDEF(Y, D);
cpu_XFER_TRYTE_INSTDEF(Y, X);

cpu_INSTDEF(TYXSP) {
	obj->SP = triad6_bct_uword_or(triad6_bct_uword_and(obj->SP, triad6_bct_uword_inv(triad6_bct_utryte2uword(bct_UTRYTE_MAX))), triad6_bct_utryte2uword(obj->X));
	cpu_WAIT_CLOCK(obj);
	obj->SP = triad6_bct_uword_or(triad6_bct_uword_shift_left(triad6_bct_utryte2uword(obj->Y), bct_TRYTE_SIZE), triad6_bct_utryte2uword(obj->X));
	cpu_WAIT_CLOCK(obj);
}

cpu_INSTDEF(TSPYX) {
	obj->X = triad6_bct_uword2utryte(obj->SP);
	cpu_WAIT_CLOCK(obj);
	obj->Y = triad6_bct_uword2utryte(triad6_bct_uword_shift_right(obj->SP, bct_TRYTE_SIZE));
	cpu_WAIT_CLOCK(obj);
}

cpu_BRANCH_INSTDEF(JMP, 1);

cpu_BRANCH_INSTDEF(BPOS, triad6_bct_getTrit(obj->F, cpu_FLAG_ZERO) == 1);
cpu_BRANCH_INSTDEF(BZRO, triad6_bct_getTrit(obj->F, cpu_FLAG_ZERO) == 0);
cpu_BRANCH_INSTDEF(BNEG, triad6_bct_getTrit(obj->F, cpu_FLAG_ZERO) == -1);
cpu_BRANCH_INSTDEF(BNPOS, triad6_bct_getTrit(obj->F, cpu_FLAG_ZERO) != 1);
cpu_BRANCH_INSTDEF(BNZRO, triad6_bct_getTrit(obj->F, cpu_FLAG_ZERO) != 0);
cpu_BRANCH_INSTDEF(BNNEG, triad6_bct_getTrit(obj->F, cpu_FLAG_ZERO) != -1);

cpu_BRANCH_INSTDEF(BOVR, triad6_bct_getTrit(obj->F, cpu_FLAG_OVERFLOW) == 1);
cpu_BRANCH_INSTDEF(BNEU, triad6_bct_getTrit(obj->F, cpu_FLAG_OVERFLOW) == 0);
cpu_BRANCH_INSTDEF(BUND, triad6_bct_getTrit(obj->F, cpu_FLAG_OVERFLOW) == -1);
cpu_BRANCH_INSTDEF(BNOVR, triad6_bct_getTrit(obj->F, cpu_FLAG_OVERFLOW) != 1);
cpu_BRANCH_INSTDEF(BNNEU, triad6_bct_getTrit(obj->F, cpu_FLAG_OVERFLOW) != 0);
cpu_BRANCH_INSTDEF(BNUND, triad6_bct_getTrit(obj->F, cpu_FLAG_OVERFLOW) != -1);

cpu_BRANCH_INSTDEF(BCRY, triad6_bct_getTrit(obj->F, cpu_FLAG_CARRY) == 1);
cpu_BRANCH_INSTDEF(BNCB, triad6_bct_getTrit(obj->F, cpu_FLAG_CARRY) == 0);
cpu_BRANCH_INSTDEF(BBRW, triad6_bct_getTrit(obj->F, cpu_FLAG_CARRY) == -1);
cpu_BRANCH_INSTDEF(BNCRY, triad6_bct_getTrit(obj->F, cpu_FLAG_CARRY) != 1);
cpu_BRANCH_INSTDEF(BNNCB, triad6_bct_getTrit(obj->F, cpu_FLAG_CARRY) != 0);
cpu_BRANCH_INSTDEF(BNBRW, triad6_bct_getTrit(obj->F, cpu_FLAG_CARRY) != -1);

cpu_RBRANCH_INSTDEF(JMP, 1);

cpu_RBRANCH_INSTDEF(BPOS, triad6_bct_getTrit(obj->F, cpu_FLAG_ZERO) == 1);
cpu_RBRANCH_INSTDEF(BZRO, triad6_bct_getTrit(obj->F, cpu_FLAG_ZERO) == 0);
cpu_RBRANCH_INSTDEF(BNEG, triad6_bct_getTrit(obj->F, cpu_FLAG_ZERO) == -1);
cpu_RBRANCH_INSTDEF(BNPOS, triad6_bct_getTrit(obj->F, cpu_FLAG_ZERO) != 1);
cpu_RBRANCH_INSTDEF(BNZRO, triad6_bct_getTrit(obj->F, cpu_FLAG_ZERO) != 0);
cpu_RBRANCH_INSTDEF(BNNEG, triad6_bct_getTrit(obj->F, cpu_FLAG_ZERO) != -1);

cpu_RBRANCH_INSTDEF(BOVR, triad6_bct_getTrit(obj->F, cpu_FLAG_OVERFLOW) == 1);
cpu_RBRANCH_INSTDEF(BNEU, triad6_bct_getTrit(obj->F, cpu_FLAG_OVERFLOW) == 0);
cpu_RBRANCH_INSTDEF(BUND, triad6_bct_getTrit(obj->F, cpu_FLAG_OVERFLOW) == -1);
cpu_RBRANCH_INSTDEF(BNOVR, triad6_bct_getTrit(obj->F, cpu_FLAG_OVERFLOW) != 1);
cpu_RBRANCH_INSTDEF(BNNEU, triad6_bct_getTrit(obj->F, cpu_FLAG_OVERFLOW) != 0);
cpu_RBRANCH_INSTDEF(BNUND, triad6_bct_getTrit(obj->F, cpu_FLAG_OVERFLOW) != -1);

cpu_RBRANCH_INSTDEF(BCRY, triad6_bct_getTrit(obj->F, cpu_FLAG_CARRY) == 1);
cpu_RBRANCH_INSTDEF(BNCB, triad6_bct_getTrit(obj->F, cpu_FLAG_CARRY) == 0);
cpu_RBRANCH_INSTDEF(BBRW, triad6_bct_getTrit(obj->F, cpu_FLAG_CARRY) == -1);
cpu_RBRANCH_INSTDEF(BNCRY, triad6_bct_getTrit(obj->F, cpu_FLAG_CARRY) != 1);
cpu_RBRANCH_INSTDEF(BNNCB, triad6_bct_getTrit(obj->F, cpu_FLAG_CARRY) != 0);
cpu_RBRANCH_INSTDEF(BNBRW, triad6_bct_getTrit(obj->F, cpu_FLAG_CARRY) != -1);

cpu_INSTDEF(CMP) {
	cpu_ADDRESS_IMM(obj);
	if (triad6_bct_getTrit(obj->F, cpu_FLAG_BALANCED) == 1) {
		bct_tryte tmp = triad6_bct_tryte_add(obj->A, obj->mdr);
		if (triad6_bct_getTrit(tmp, bct_TRYTE_SIZE) != 0) {
			triad6_bct_setTrit(obj->F, cpu_FLAG_CARRY, triad6_bct_getTrit(tmp, bct_TRYTE_SIZE));
		}
		if (triad6_bct_tryte_value(tmp) > triad6_bct_tryte_value(obj->A)) {
			triad6_bct_setTrit(obj->F, cpu_FLAG_OVERFLOW, -1);
		}
		else {
			triad6_bct_setTrit(obj->F, cpu_FLAG_OVERFLOW, 0);
		}
		triad6_bct_setTrit(obj->F, cpu_FLAG_ZERO, cpu_SIGN(triad6_bct_tryte_value(tmp)));
	}
	else if (triad6_bct_getTrit(obj->F, cpu_FLAG_BALANCED) == -1) {
		bct_utryte tmp = triad6_bct_utryte_add(obj->A, triad6_bct_rollTryte(triad6_bct_utryte_add(triad6_bct_utryte_inv(obj->mdr), triad6_bct_utryte_convert(1))));
		if (triad6_bct_getUTrit(tmp, bct_TRYTE_SIZE) != 0) {
			triad6_bct_setUTrit(obj->F, cpu_FLAG_CARRY, triad6_bct_getUTrit(tmp, bct_TRYTE_SIZE));
		}
		if (triad6_bct_utryte_value(tmp) > triad6_bct_utryte_value(obj->A)) {
			triad6_bct_setTrit(obj->F, cpu_FLAG_OVERFLOW, -1);
		}
		triad6_bct_setTrit(obj->F, cpu_FLAG_ZERO, cpu_SIGN(triad6_bct_utryte_value(tmp)));
	}
}

cpu_SET_FLAG_INSTDEF(OU, OVERFLOW);
cpu_SET_FLAG_INSTDEF(CB, CARRY);
cpu_SET_FLAG_INSTDEF(I, INTERRUPT);
cpu_SET_FLAG_INSTDEF(BU, BALANCED);

static cpu_instruction_cb cpu_opcodes[bct_UTRYTE_MAX + 1];

#define OPCODE(str, name) cpu_opcodes[triad6_bct_utryte_value(triad6_bct_utryte_septemvigits(#str))] = cpu_GET_INST(name)

void triad6_cpu_init(cpu_state *obj) {
	for (size_t op = 0; op < bct_UTRYTE_MAX + 1; op++) {
		cpu_opcodes[op] = cpu_GET_INST(invalid);
	}
	
	OPCODE(00, NOP);
	
	OPCODE(20, LDA_IMM);
	OPCODE(21, LDC_IMM);
	OPCODE(22, LDD_IMM);
	OPCODE(23, LDX_IMM);
	OPCODE(24, LDY_IMM);
	
	OPCODE(25, LDA_ABS);
	OPCODE(26, LDC_ABS);
	OPCODE(27, LDD_ABS);
	OPCODE(28, LDX_ABS);
	OPCODE(29, LDY_ABS);
	
	OPCODE(2A, LDA_ABX);
	OPCODE(2B, LDC_ABX);
	OPCODE(2C, LDD_ABX);
	OPCODE(2D, LDY_ABX);
	
	OPCODE(2E, LDA_ABY);
	OPCODE(2F, LDC_ABY);
	OPCODE(2G, LDD_ABY);
	OPCODE(2H, LDX_ABY);
	
	OPCODE(2I, LDA_IND);
	OPCODE(2J, LDD_IND);
	
	OPCODE(2K, LDA_IDX);
	OPCODE(2L, LDD_IDX);
	
	OPCODE(2M, LDA_IDY);
	OPCODE(2N, LDD_IDY);
	
	OPCODE(30, STA_ABS);
	OPCODE(31, STC_ABS);
	OPCODE(32, STD_ABS);
	OPCODE(33, STX_ABS);
	OPCODE(34, STY_ABS);
	
	OPCODE(35, STA_ABX);
	OPCODE(36, STC_ABX);
	OPCODE(37, STD_ABX);
	OPCODE(38, STY_ABX);
	
	OPCODE(39, STA_ABY);
	OPCODE(3A, STC_ABY);
	OPCODE(3B, STD_ABY);
	OPCODE(3C, STX_ABY);
	
	OPCODE(40, ADD_IMM);
	OPCODE(41, ADD_ABS);
	OPCODE(42, ADD_ABX);
	OPCODE(43, ADD_ABY);
	OPCODE(44, ADD_IND);
	OPCODE(45, ADD_IDX);
	OPCODE(46, ADD_IDY);
	OPCODE(47, ADDD);
	
	OPCODE(48, ADDC_IMM);
	OPCODE(49, ADDC_ABS);
	OPCODE(4A, ADDC_ABX);
	OPCODE(4B, ADDC_ABY);
	OPCODE(4C, ADDC_IND);
	OPCODE(4D, ADDC_IDX);
	OPCODE(4E, ADDC_IDY);
	OPCODE(4F, ADDDC);
	
	OPCODE(4G, ADDB_IMM);
	OPCODE(4H, ADDB_ABS);
	OPCODE(4I, ADDB_ABX);
	OPCODE(4J, ADDB_ABY);
	OPCODE(4K, ADDB_IND);
	OPCODE(4L, ADDB_IDX);
	OPCODE(4M, ADDB_IDY);
	OPCODE(4N, ADDDB);
	
	OPCODE(50, SUB_IMM);
	
	OPCODE(80, TCA);
	OPCODE(81, TDA);
	OPCODE(82, TXA);
	OPCODE(83, TYA);
	
	OPCODE(84, TAC);
	OPCODE(85, TDC);
	OPCODE(86, TXC);
	OPCODE(87, TYC);
	
	OPCODE(88, TAD);
	OPCODE(89, TCD);
	OPCODE(8A, TXD);
	OPCODE(8B, TYD);
	
	OPCODE(8C, TAX);
	OPCODE(8D, TCX);
	OPCODE(8E, TDX);
	OPCODE(8F, TYX);
	
	OPCODE(8G, TAY);
	OPCODE(8H, TCY);
	OPCODE(8I, TDY);
	OPCODE(8J, TXY);
	
	OPCODE(8K, TYXSP);
	OPCODE(8L, TSPYX);
	
	OPCODE(C0, JMP);
	
	OPCODE(C1, BPOS);
	OPCODE(C2, BZRO);
	OPCODE(C3, BNEG);
	OPCODE(C4, BNPOS);
	OPCODE(C5, BNZRO);
	OPCODE(C6, BNNEG);
	
	OPCODE(C7, BOVR);
	OPCODE(C8, BNEU);
	OPCODE(C9, BUND);
	OPCODE(CA, BNOVR);
	OPCODE(CB, BNNEU);
	OPCODE(CC, BNUND);
	
	OPCODE(CD, BCRY);
	OPCODE(CE, BNCB);
	OPCODE(CF, BBRW);
	OPCODE(CG, BNCRY);
	OPCODE(CH, BNNCB);
	OPCODE(CI, BNBRW);
	
	OPCODE(D0, RJMP);
	
	OPCODE(D1, RBPOS);
	OPCODE(D2, RBZRO);
	OPCODE(D3, RBNEG);
	OPCODE(D4, RBNPOS);
	OPCODE(D5, RBNZRO);
	OPCODE(D6, RBNNEG);
	
	OPCODE(D7, RBOVR);
	OPCODE(D8, RBNEU);
	OPCODE(D9, RBUND);
	OPCODE(DA, RBNOVR);
	OPCODE(DB, RBNNEU);
	OPCODE(DC, RBNUND);
	
	OPCODE(DD, RBCRY);
	OPCODE(DE, RBNCB);
	OPCODE(DF, RBBRW);
	OPCODE(DG, RBNCRY);
	OPCODE(DH, RBNNCB);
	OPCODE(DI, RBNBRW);
	
	OPCODE(E0, CMP);
	
	OPCODE(P0, SETOU);
	OPCODE(P1, SETCB);
	OPCODE(P2, SETI);
	OPCODE(P3, SETBU);
	
	obj->fetch = true;
}

#undef OPCODE

static bool stopCPU = true;

static void cpu_loop(cpu_state *obj) {
	while ((util_perfCounter() - obj->startClockTime) < (PERF_COUNTER_UNITS / FRAMERATE)) {
		if(obj->fetch) {
			obj->PC = obj->instPtr;
			cpu_EXEC_INST(rdpc);
			obj->ir = triad6_bct_rollTryte(obj->mdr);
			obj->fetch = false;
		}
		else if (!setjmp(renderEnv)) {
			obj->lastClockTime = util_perfCounter();
			while ((util_perfCounter() - obj->lastClockTime) < obj->clockPeriod) {
				cpu_opcodes[triad6_bct_utryte_value(obj->ir)](obj);
				obj->fetch = true;
			}
		}
	}
}

void triad6_cpu_execute(cpu_state *obj) {
	obj->lastClockTime = util_perfCounter();
	obj->startClockTime = util_perfCounter();
	
	if (stopCPU) {
		if (!setjmp(renderEnv)) {
			stopCPU = false;
			cpu_loop(obj);
		}
	}
	else {
		if (!setjmp(renderEnv)) {
			cpu_loop(obj);
		}
	}
}

void triad6_cpu_quit(cpu_state *obj) {
	obj->lastClockTime = util_perfCounter();
	obj->startClockTime = util_perfCounter();
	
	if (!setjmp(renderEnv)) {
		stopCPU = true;
		cpu_loop(obj);
	}
}