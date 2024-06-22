#include <stdbool.h>
#include <stdlib.h>
#include <setjmp.h>

#include "cpu.h"
#include "utility.h"

static jmp_buf cpuEnv;

// this determines how long to run the CPU for on each frame
// a value of 81 (to clear overhead for the BVS1) gets me pretty close to a solid 60 FPS on my computer
#define FRAMERATE 81

#define CONCAT_(one, two, three, four) one##two##three##four

#define cpu_INSTDEF(name) CONCAT_(static void cpu_inst_,name,,)(cpu_state *obj)
#define cpu_GET_INST(name) CONCAT_(cpu_inst_,name,,)
#define cpu_EXEC_INST(name) cpu_GET_INST(name)(obj)

#define cpu_SETUP_INST() uint8_t cycles = 1
#define cpu_WAIT_CLOCK(op) do { \
	if (obj->thisCycle == cycles) { \
		do op while (0); \
		longjmp(cpuEnv, 1); \
	} \
	cycles++; \
} while (0) \

#define cpu_INDEX_CONVERT(obj, index) ((triad6_bct_getTrit(obj->F, cpu_FLAG_BALANCED) == 1) ? triad6_bct_tryte2uword(index) : triad6_bct_utryte2uword(index))
#define cpu_INCR_UWORD(reg) reg = triad6_bct_uword_add((reg), triad6_bct_uword_convert(1))

// No OPeration
cpu_INSTDEF(NOP) {
	cpu_SETUP_INST();
	cpu_WAIT_CLOCK({
		obj->fetch = true;
	});
}

// Read a tryte from the address pointed to by PC
cpu_INSTDEF(rdpc) {
	obj->mar = obj->instPtr;
	obj->mdr = obj->readTryte(obj->mar);
	cpu_INCR_UWORD(obj->instPtr);
}

// Read a tryte from the address pointed to by MAR
cpu_INSTDEF(rdmar) {
	obj->mdr = obj->readTryte(obj->mar);
}

// Read a tryte from the address pointed to by MAR, then increment MAR
cpu_INSTDEF(rdmarinc) {
	obj->mdr = obj->readTryte(obj->mar);
	cpu_INCR_UWORD(obj->mar);
}

// Write a tryte to the address pointed to by MAR
cpu_INSTDEF(wrmar) {
	obj->writeTryte(obj->mar, obj->mdr);
}

#define cpu_ADDRESS_IMM(obj) \
	cpu_SETUP_INST(); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdpc); \
	})

#define cpu_ADDRESS_ABS(obj) \
	cpu_SETUP_INST(); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdpc); \
		(obj)->tmp = triad6_bct_utryte2uword((obj)->mdr); \
	}); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdpc); \
		(obj)->mar = triad6_bct_uword_add((obj)->tmp, triad6_bct_uword_shift_left((obj)->mdr, bct_TRYTE_SIZE)); \
	}); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdmar); \
	})

#define cpu_ADDRESS_ABX(obj) \
	cpu_SETUP_INST(); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdpc); \
		(obj)->tmp = triad6_bct_utryte2uword((obj)->mdr); \
	}); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdpc); \
		(obj)->mar = triad6_bct_uword_add(triad6_bct_uword_add((obj)->tmp, triad6_bct_uword_shift_left((obj)->mdr, bct_TRYTE_SIZE)), cpu_INDEX_CONVERT(obj, (obj)->X)); \
	}); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdmar); \
	})

#define cpu_ADDRESS_ABY(obj) \
	cpu_SETUP_INST(); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdpc); \
		(obj)->tmp = triad6_bct_utryte2uword((obj)->mdr); \
	}); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdpc); \
		(obj)->mar = triad6_bct_uword_add(triad6_bct_uword_add((obj)->tmp, triad6_bct_uword_shift_left((obj)->mdr, bct_TRYTE_SIZE)), cpu_INDEX_CONVERT(obj, (obj)->Y)); \
	}); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdmar); \
	})

#define cpu_ADDRESS_IND(obj) \
	cpu_SETUP_INST(); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdpc); \
		(obj)->tmp = triad6_bct_utryte2uword((obj)->mdr); \
	}); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdpc); \
		(obj)->mar = triad6_bct_uword_add((obj)->tmp, triad6_bct_uword_shift_left((obj)->mdr, bct_TRYTE_SIZE)); \
	}); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdmarinc); \
		(obj)->tmp = triad6_bct_utryte2uword((obj)->mdr); \
	}); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdmar); \
		(obj)->mar = triad6_bct_uword_add((obj)->tmp, triad6_bct_uword_shift_left((obj)->mdr, bct_TRYTE_SIZE)); \
	}); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdmar); \
	})

#define cpu_ADDRESS_IDX(obj) \
	cpu_SETUP_INST(); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdpc); \
		(obj)->tmp = triad6_bct_utryte2uword((obj)->mdr); \
	}); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdpc); \
		(obj)->mar = triad6_bct_uword_add((obj)->tmp, triad6_bct_uword_shift_left((obj)->mdr, bct_TRYTE_SIZE)); \
	}); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdmarinc); \
		(obj)->tmp = triad6_bct_utryte2uword((obj)->mdr); \
	}); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdmar); \
		(obj)->mar = triad6_bct_uword_add(triad6_bct_uword_add((obj)->tmp, triad6_bct_uword_shift_left((obj)->mdr, bct_TRYTE_SIZE)), cpu_INDEX_CONVERT(obj, (obj)->X)); \
	}); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdmar); \
	})

#define cpu_ADDRESS_IDY(obj) \
	cpu_SETUP_INST(); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdpc); \
		(obj)->tmp = triad6_bct_utryte2uword((obj)->mdr); \
	}); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdpc); \
		(obj)->mar = triad6_bct_uword_add((obj)->tmp, triad6_bct_uword_shift_left((obj)->mdr, bct_TRYTE_SIZE)); \
	}); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdmarinc); \
		(obj)->tmp = triad6_bct_utryte2uword((obj)->mdr); \
	}); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdmar); \
		(obj)->mar = triad6_bct_uword_add(triad6_bct_uword_add((obj)->tmp, triad6_bct_uword_shift_left((obj)->mdr, bct_TRYTE_SIZE)), cpu_INDEX_CONVERT(obj, (obj)->Y)); \
	}); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdmar); \
	})

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
	obj->fetch = true; \
}

#define cpu_ST_INSTDEF(reg, mode) cpu_INSTDEF(CONCAT_(ST,reg,_,mode)) { \
	CONCAT_(cpu_ADDRESS_,mode,,)(obj); \
	obj->mdr = obj->reg; \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(wrmar); \
		obj->fetch = true; \
	}); \
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
		triad6_bct_setTrit(obj->F, cpu_FLAG_ZERO, (triad6_bct_utryte_value(tmp) > (bct_UTRYTE_MAX / 2)) ? -1 : cpu_SIGN(triad6_bct_utryte_value(tmp))); \
		obj->A = tmp; \
	} \
	obj->fetch = true; \
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
		triad6_bct_setTrit(obj->F, cpu_FLAG_ZERO, (triad6_bct_utryte_value(tmp) > (bct_UTRYTE_MAX / 2)) ? -1 : cpu_SIGN(triad6_bct_utryte_value(tmp))); \
		obj->A = tmp; \
	} \
	obj->fetch = true; \
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
		triad6_bct_setTrit(obj->F, cpu_FLAG_ZERO, (triad6_bct_utryte_value(tmp) > (bct_UTRYTE_MAX / 2)) ? -1 : cpu_SIGN(triad6_bct_utryte_value(tmp))); \
		obj->A = tmp; \
	} \
	obj->fetch = true; \
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
		triad6_bct_setTrit(obj->F, cpu_FLAG_ZERO, (triad6_bct_utryte_value(tmp) > (bct_UTRYTE_MAX / 2)) ? -1 : cpu_SIGN(triad6_bct_utryte_value(tmp))); \
		obj->A = tmp; \
	} \
	obj->fetch = true; \
}

#define cpu_XFER_TRYTE_INSTDEF(dst, src) cpu_INSTDEF(CONCAT_(T,src,dst,)) { \
	cpu_SETUP_INST(); \
	cpu_WAIT_CLOCK({ \
		obj->dst = obj->src; \
		obj->fetch = true; \
	}); \
}

#define cpu_BRANCH_INSTDEF(opName, condExpr) cpu_INSTDEF(opName) { \
	cpu_SETUP_INST(); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdpc); \
		(obj)->tmp = triad6_bct_utryte2uword((obj)->mdr); \
	}); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdpc); \
		(obj)->tmp = triad6_bct_uword_add((obj)->tmp, triad6_bct_uword_shift_left((obj)->mdr, bct_TRYTE_SIZE)); \
		if (!(condExpr)) obj->fetch = true; \
	}); \
	cpu_WAIT_CLOCK({ \
		obj->instPtr = obj->tmp; \
		obj->fetch = true; \
	}); \
}

#define cpu_RBRANCH_INSTDEF(opName, condExpr) cpu_INSTDEF(CONCAT_(R,opName,,)) { \
	cpu_SETUP_INST(); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdpc); \
		if (!(condExpr)) obj->fetch = true; \
	}); \
	cpu_WAIT_CLOCK({ \
		if (triad6_bct_tryte_value(obj->mdr) < 0) { \
			obj->instPtr = triad6_bct_uword_add(obj->PC, triad6_bct_uword_add(triad6_bct_uword_inv(triad6_bct_tryte2uword(triad6_bct_tryte_inv(obj->mdr))), triad6_bct_uword_convert(1))); \
		} \
		else { \
			obj->instPtr = triad6_bct_uword_add(obj->PC, triad6_bct_tryte2uword(obj->mdr)); \
		} \
		obj->fetch = true; \
	}); \
}

#define cpu_SET_FLAG_INSTDEF(flagName, flagEnum) cpu_INSTDEF(CONCAT_(SET,flagName,,)) { \
	cpu_SETUP_INST(); \
	cpu_WAIT_CLOCK({ \
		cpu_EXEC_INST(rdpc); \
		triad6_bct_setTrit(obj->F, CONCAT_(cpu_FLAG_,flagEnum,,), triad6_bct_getTrit(obj->mdr, 0)); \
		obj->fetch = true; \
	}); \
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

	obj->fetch = true;
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

	obj->fetch = true;
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
		triad6_bct_setTrit(obj->F, cpu_FLAG_ZERO, (triad6_bct_utryte_value(tmp) > (bct_UTRYTE_MAX / 2)) ? -1 : cpu_SIGN(triad6_bct_utryte_value(tmp)));
		obj->A = tmp;
	}

	obj->fetch = true;
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
	cpu_SETUP_INST();
	cpu_WAIT_CLOCK({
		obj->SP = triad6_bct_uword_add(triad6_bct_uword_and(obj->SP, triad6_bct_uword_inv(triad6_bct_uword_convert(bct_UTRYTE_MAX))), triad6_bct_utryte2uword(obj->X));
	});
	cpu_WAIT_CLOCK({
		obj->SP = triad6_bct_uword_add(triad6_bct_uword_shift_left(triad6_bct_utryte2uword(obj->Y), bct_TRYTE_SIZE), triad6_bct_utryte2uword(obj->X));
		obj->fetch = true;
	});
}

cpu_INSTDEF(TSPYX) {
	cpu_SETUP_INST();
	cpu_WAIT_CLOCK({
		obj->X = triad6_bct_uword2utryte(obj->SP);
	});
	cpu_WAIT_CLOCK({
		obj->Y = triad6_bct_uword2utryte(triad6_bct_uword_shift_right(obj->SP, bct_TRYTE_SIZE));
		obj->fetch = true;
	});
}

cpu_BRANCH_INSTDEF(JMP, true);

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

cpu_RBRANCH_INSTDEF(JMP, true);

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
		bct_tryte tmp = triad6_bct_tryte_sub(obj->A, obj->mdr);
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
		bct_utryte tmp = triad6_bct_utryte_sub(obj->A, obj->mdr);
		if (triad6_bct_getUTrit(tmp, bct_TRYTE_SIZE) != 0) {
			triad6_bct_setUTrit(obj->F, cpu_FLAG_CARRY, triad6_bct_getUTrit(tmp, bct_TRYTE_SIZE));
		}
		if (triad6_bct_utryte_value(tmp) > triad6_bct_utryte_value(obj->A)) {
			triad6_bct_setTrit(obj->F, cpu_FLAG_OVERFLOW, -1);
		}
		else {
			triad6_bct_setTrit(obj->F, cpu_FLAG_OVERFLOW, 0);
		}
		
		triad6_bct_setTrit(obj->F, cpu_FLAG_ZERO, (triad6_bct_utryte_value(tmp) > ((bct_UTRYTE_MAX - 1) / 2)) ? -1 : cpu_SIGN(triad6_bct_utryte_value(tmp)));
	}
	obj->fetch = true;
}

cpu_SET_FLAG_INSTDEF(OU, OVERFLOW);
cpu_SET_FLAG_INSTDEF(CB, CARRY);
cpu_SET_FLAG_INSTDEF(I, INTERRUPT);
cpu_SET_FLAG_INSTDEF(BU, BALANCED);

static cpu_instruction_cb cpu_opcodes[bct_UTRYTE_MAX + 1];

#define OPCODE(str, name) cpu_opcodes[triad6_bct_utryte_value(triad6_bct_utryte_septemvigits(#str))] = cpu_GET_INST(name)

void triad6_cpu_init(cpu_state *obj) {
	for (size_t op = 0; op < bct_UTRYTE_MAX + 1; op++) {
		cpu_opcodes[op] = cpu_GET_INST(NOP);
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
	
	obj->thisCycle = 0;
	obj->fetch = true;
}

#undef OPCODE

static bool stopCPU = true;

static void cpu_loop(cpu_state *obj) {
	// note: commenting out this outer loop will make the CPU run at 60 Hz -- better for testing
	while ((triad6_util_perfCounter() - obj->startClockTime) < (PERF_COUNTER_UNITS / FRAMERATE)) {
		obj->lastClockTime = triad6_util_perfCounter();
		
		if(obj->fetch) {
			obj->PC = obj->instPtr;
			cpu_EXEC_INST(rdpc);
			obj->ir = triad6_bct_rollTryte(obj->mdr);
			obj->fetch = false;
			obj->thisCycle = 0;
			do {} while ((triad6_util_perfCounter() - obj->lastClockTime) < obj->clockPeriod);
		}
		else if (!setjmp(cpuEnv)) {
			++obj->thisCycle;
			cpu_opcodes[triad6_bct_utryte_value(obj->ir)](obj);
			do {} while ((triad6_util_perfCounter() - obj->lastClockTime) < obj->clockPeriod);
		}
	}
}

void triad6_cpu_execute(cpu_state *obj) {
	obj->startClockTime = triad6_util_perfCounter();
	
	if (stopCPU) {
		if (!setjmp(cpuEnv)) {
			stopCPU = false;
			cpu_loop(obj);
		}
	}
	else {
		if (!setjmp(cpuEnv)) {
			cpu_loop(obj);
		}
	}
}

void triad6_cpu_quit(cpu_state *obj) {
	obj->startClockTime = triad6_util_perfCounter();
	
	if (!setjmp(cpuEnv)) {
		stopCPU = true;
		cpu_loop(obj);
	}
}