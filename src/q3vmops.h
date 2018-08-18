/*
Triseism - Standlone interpreter of Quake III Virtual Machine
Copyright 2003  PhaethonH <phaethon@linux.ucla.edu>

Permission granted to copy, modify, distribute, or otherwise use this code,
provided this copyright notice remains intact
*/

/*
 vm core: Q3VM opcodes.
*/

#ifndef _Q3VMOPS_H_
#define _Q3VMOPS_H_

#include "q3vm.h"

/* mirrored from q3asm.c */

#define OP(n) VMOP_##n
enum {
  OP(UNDEF) = 0,
  OP(IGNORE),  /* no-op */
  OP(BREAK),   /* ??? */
  OP(ENTER),   /* Begin subroutine. */
  OP(LEAVE),   /* End subroutine. */
  OP(CALL),    /* Call subroutine. */
  OP(PUSH),    /* push to stack. */
  OP(POP),     /* discard top-of-stack. */
  OP(CONST),   /* load constant to stack. */
  OP(LOCAL),   /* get local variable. */
  OP(JUMP),    /* unconditional jump. */
  OP(EQ),      /* compare integers, jump if equal. */
  OP(NE),      /* compare integers, jump if not equal. */
  OP(LTI),     /* compare integers, jump if less-than. */
  OP(LEI),     /* compare integers, jump if less-than-or-equal. */
  OP(GTI),     /* compare integers, jump if greater-than. */
  OP(GEI),     /* compare integers, jump if greater-than-or-equal. */
  OP(LTU),     /* compare unsigned integers, jump if less-than */
  OP(LEU),     /* compare unsigned integers, jump if less-than-or-equal */
  OP(GTU),     /* compare unsigned integers, jump if greater-than */
  OP(GEU),     /* compare unsigned integers, jump if greater-than-or-equal */
  OP(EQF),     /* compare floats, jump if equal */
  OP(NEF),     /* compare floats, jump if not-equal */
  OP(LTF),     /* compare floats, jump if less-than */
  OP(LEF),     /* compare floats, jump if less-than-or-equal */
  OP(GTF),     /* compare floats, jump if greater-than */
  OP(GEF),     /* compare floats, jump if greater-than-or-equal */
  OP(LOAD1),   /* load 1-byte from memory */
  OP(LOAD2),   /* load 2-byte from memory */
  OP(LOAD4),   /* load 4-byte from memory */
  OP(STORE1),  /* store 1-byte to memory */
  OP(STORE2),  /* store 2-byte to memory */
  OP(STORE4),  /* store 4-byte to memory */
  OP(ARG),     /* marshal argument */
  OP(BLOCK_COPY), /* block copy... */
  OP(SEX8),    /* Pedophilia */
  OP(SEX16),   /* Sign-Extend 16-bit */
  OP(NEGI),    /* Negate integer. */
  OP(ADD),     /* Add integers (two's complement). */
  OP(SUB),     /* Subtract integers (two's complement). */
  OP(DIVI),    /* Divide signed integers. */
  OP(DIVU),    /* Divide unsigned integers. */
  OP(MODI),    /* Modulus (signed). */
  OP(MODU),    /* Modulus (unsigned). */
  OP(MULI),    /* Multiply signed integers. */
  OP(MULU),    /* Multiply unsigned integers. */
  OP(BAND),    /* Bitwise AND */
  OP(BOR),     /* Bitwise OR */
  OP(BXOR),    /* Bitwise eXclusive-OR */
  OP(BCOM),    /* Bitwise COMplement */
  OP(LSH),     /* Left-shift */
  OP(RSHI),    /* Right-shift (algebraic; preserve sign) */
  OP(RSHU),    /* Right-shift (bitwise; ignore sign) */
  OP(NEGF),    /* Negate float */
  OP(ADDF),    /* Add floats */
  OP(SUBF),    /* Subtract floats */
  OP(DIVF),    /* Divide floats */
  OP(MULF),    /* Multiply floats */
  OP(CVIF),    /* Convert to integer from float */
  OP(CVFI),    /* Convert to float from integer */


  OP(NUM_OPS),
};

extern const char *opnames[OP(NUM_OPS)];

typedef int (*opfunc)(struct q3vm_t *self, vmword parm);
extern opfunc optable[OP(NUM_OPS)];

#endif /* _Q3VMOPS_H_ */
