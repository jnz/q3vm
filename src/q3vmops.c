/*
Triseism - Standlone interpreter of Quake III Virtual Machine
Copyright 2003  PhaethonH <phaethon@linux.ucla.edu>

Permission granted to copy, modify, distribute, or otherwise use this code,
provided this copyright notice remains intact
*/

/*
  vm core: Q3VM opcodes.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "q3vmops.h"


/*
TODO:
 Copy args from marshal-space to local space.
*/


/*
 Q3VM opcodes.
*/

#undef _THIS
#define _THIS q3vm_t *self

#undef OP
#define OP(n) q3vm_op##n (_THIS, vmword parm)

//#define OPPARM (self->rom[self->PC].parm)
#define OPPARM (parm)

#define R0 (self->r[0])
#define R1 (self->r[1])
#define R2 (self->r[2])
#define CMF (self->cm)


int OP(UNDEF)
{
  /* Die horribly. */
  return -1;
}

int OP(IGNORE)
{
  /* NOP */
  return 0;
}

int OP(BREAK)
{
  /* Usage never spotted. */
  /* Die horribly? */
  return -1;
}

/*
Stack on entering...

no locals: ENTER 8
1 words locals: ENTER 16
2 words locals: ENTER 20
3 words locals: ENTER 24
 etc.

address of argument:
 ADDRFP4 v  =>  OP_LOCAL (16 + currentLocals + currentArgs + v)
address of local:
 ADDRLP4 v  =>  OP_LOCAL (8 + currentArgs + v)

      RP [        ]   ??? (oldPC?)
         [        ]   ???
         [        ]  \
            ...        > locals (args marshalling)
         [        ]  /
         [        ]  \
            ...        > locals
         [        ]  /     (ADDRLP4 v  =>  OP_LOCAL (8 + currentArgs + v))
(oldRP?) [        ]   ???
         [        ]   ???
         [        ]   (my args?)
            ...
         [        ]
*/

int OP(ENTER)  /* ??? */
{
  R2.I4 = self->RP;
  R1 = OPPARM;
//crumb("ENTER with shift %d B (PC=%d, DP=%d, RP=%d)\n", R1.I4, self->PC, self->DP, self->RP);
  R0.I4 = 0;
  while (R1.I4 > (2 * sizeof(vmword)))
    {
      q3vm_rpush(self, R0);  /* init zero */
      R1.I4 -= sizeof(vmword);
    }
//  R0.I4 = self->PC; q3vm_rpush(self, R0);  /* PC? */
  q3vm_to_r(self);  /* return-PC */
  R0.I4 = 0; q3vm_rpush(self, R0);  /* I dunno */
//crumb("ENTER done with (PC=%d, DP=%d, RP=%d)\n", self->PC, self->DP, self->RP);
  return 0;
}

int OP(LEAVE)  /* ??? */
{
  R1 = OPPARM;
//crumb("LEAVE with unshift %d B (PC=%d, DP=%d, RP=%d)\n", R1.I4, self->PC, self->DP, self->RP);
  R0 = q3vm_rdrop(self);  /* I dunno */
  R1.I4 -= sizeof(vmword);
  R0 = q3vm_rdrop(self);  /* PC? */
  self->PC = R0.I4;
  R1.I4 -= sizeof(vmword);
  while (R1.I4 > 0)
    {
      q3vm_rdrop(self);
      R1.I4 -= sizeof(vmword);
    }
//crumb(" LEAVE done (PC=%d, DP=%d, RP=%d)\n", self->PC, self->DP, self->RP);
  return 0;
}

int OP(CALL)  /* Call subroutine. */
{
  R0 = q3vm_drop(self);   /* jump address. */
  R1.I4 = self->PC;
  q3vm_push(self, R1);    /* return address */
//crumb("(PC=%d) Making procedure call to %d, will return to %d (DP=%d, RP=%d)\n", self->PC, R0.I4, R1.I4, self->DP, self->RP);
  self->PC = R0.I4;
  return 0;
}

int OP(PUSH)  /* [DP] <- 0; DP++ */
{
  R0.I4 = 0;
  q3vm_push(self, R0);
  return 0;
}

int OP(POP)  /* DP-- */
{
  q3vm_drop(self);
  return 0;
}

int OP(CONST)  /* [DP] <- parm; DP++ */
{
  q3vm_push(self, OPPARM);
//crumb("Pushing constant %08X\n", OPPARM.I4);
  return 0;
}

int OP(LOCAL)  /* [DP] <- [RP-n] */
{
  vmword w;

  R0 = OPPARM;
//  R1.I4 = q3vm_local(self, R0.I4);
  R1.I4 = self->RP + R0.I4;
//crumb("Getting local %d => [%d]\n", R0.I4, R1.I4);
  q3vm_push(self, R1);
  return 0;
}

int OP(JUMP)   /* PC <- [DP] */
{
  R0 = q3vm_drop(self);
//  q3vm_jump(self, R0.I4);
//crumb("JUMPing to %d (0x%08X)\n", R0.I4, R0.U4);
  self->PC = R0.I4;
  return 0;
}

#if 0
int OP(EQ)     /* if [DP] == [DP-1] then PC <- parm; DP <- DP-2 */
{
  R1 = q3vm_drop(self);
  R0 = q3vm_drop(self);
  CMF = (R0.I4 == R1.I4);
  if (CMF) self->PC = R0.I4; //q3vm_jump(self, OPPARM);
  return 0;
}
#endif /* 0 */

#if 0
#define CMP(vmtype, op) { R1 = q3vm_drop(self); R0 = q3vm_drop(self); \
  CMF = (R0.vmtype op R1.vmtype); if (CMF) self->PC = OPPARM.I4 /* q3vm_jump(self, OPPARM) */; crumb(" Comparison %s on %d vs %d, cond-jump %d\n", #op, R0.I4, R1.I4, OPPARM.I4); return 0; }
#else
#define CMP(vmtype, op) { R1 = q3vm_drop(self); R0 = q3vm_drop(self); \
  CMF = (R0.vmtype op R1.vmtype); if (CMF) self->PC = OPPARM.I4; return 0; }
#endif /* 0 */

int OP(EQ)     /* if [DP] == [DP-1] then PC <- parm; DP <- DP-2 */
  CMP(I4, ==)

int OP(NE)   /* if [DP] == [DP-1] then PC <- parm; DP <- DP-2 */
  CMP(I4, !=)

int OP(LTI)  /* if [DP] < [DP-1] then PC <- parm; DP <- DP-2 */
  CMP(I4, <)

int OP(LEI)  /* if [DP] <= [DP-1] then PC <- parm; DP <- DP-2 */
  CMP(I4, <=)

int OP(GTI)  /* if [DP] > [DP-1] then PC <- parm; DP <- DP-2 */
  CMP(I4, >)

int OP(GEI)  /* if [DP] >= [DP-1] then PC <- parm; DP <- DP-2 */
  CMP(I4, >=)

int OP(LTU)  /* if [DP] < [DP-1] then PC <- parm; DP <- DP-2 */
  CMP(U4, <)

int OP(LEU)  /* if [DP] <= [DP-1] then PC <- parm; DP <- DP-2 */
  CMP(U4, <=)

int OP(GTU)  /* if [DP] > [DP-1] then PC <- parm; DP <- DP-2 */
  CMP(U4, >)

int OP(GEU)  /* if [DP] >= [DP-1] then PC <- parm; DP <- DP-2 */
  CMP(U4, >=)

int OP(EQF)  /* if [DP] == [DP-1] then PC <- parm; DP <- DP-2 */
  CMP(F4, ==)

int OP(NEF)  /* if [DP] != [DP-1] then PC <- parm; DP <- DP-2 */
  CMP(F4, !=)

int OP(LTF)  /* if [DP] < [DP-1] then PC <- parm; DP <- DP-2 */
  CMP(F4, <)

int OP(LEF)  /* if [DP] <= [DP-1] then PC <- parm; DP <- DP-2 */
  CMP(F4, <=)

int OP(GTF)  /* if [DP] > [DP-1] then PC <- parm; DP <- DP-2 */
  CMP(F4, >)

int OP(GEF)  /* if [DP] >= [DP-1] then PC <- parm; DP <- DP-2 */
  CMP(F4, >=)


int OP(LOAD1)  /* [DP] <- [[DP]] */
{
  R1 = q3vm_drop(self);
  R0.I4 = q3vm_get_U1(self, R1.I4);
//crumb("Retrieving octet %d from [%d]\n", R0.I4, R1.I4);
  q3vm_push(self, R0);
  return 0;
}

int OP(LOAD2)  /* [DP] <- [[DP]] */
{
  R1 = q3vm_drop(self);
  R0.I4 = q3vm_get_U2(self, R1.I4);
//crumb("Retrieving short %d from [%d]\n", R0.I4, R1.I4);
  q3vm_push(self, R0);
  return 0;
}

int OP(LOAD4)  /* [DP] <- [[DP]] */
{
  R1 = q3vm_drop(self);
  R0.I4 = q3vm_get_I4(self, R1.I4);
//crumb("LOAD4 from [%d] => %d\n", R1.I4, R0.I4);
  q3vm_push(self, R0);
  return 0;
}

int OP(STORE1) /* [DP-1] <- [DP]; DP <- DP-2 */
{
  R1 = q3vm_drop(self); /* value */
  R0 = q3vm_drop(self); /* destination */
//crumb("Storing octet %d to [%d]\n", R1.U4, R0.U4);
  q3vm_set_U1(self, R0.I4, (R1.U4 & 0xFF));
  return 0;
}

int OP(STORE2) /* [DP-1] <- [DP]; DP <- DP-2 */
{
  R1 = q3vm_drop(self); /* value */
  R0 = q3vm_drop(self); /* destination */
//crumb("Storing short %d to [%d]\n", R1.U4, R0.U4);
  q3vm_set_U2(self, R0.I4, (R1.U4 & 0xFFFF));
  return 0;
}

int OP(STORE4) /* [DP-1] <- [DP]; DP <- DP-2 */
{
  R1 = q3vm_drop(self); /* value */
  R0 = q3vm_drop(self); /* destination */
//crumb("Storing word %d to [%d]\n", R1.I4, R0.I4);
  q3vm_set_I4(self, R0.I4, R1.I4);
  return 0;
}

int OP(ARG)    /* Marshal TOS to to-call argument list */
{
  R0 = q3vm_drop(self);
//  q3vm_argument(self, R0);
//  q3vm_set_word(self, self->AP, R0);
//  self->AP++;
//  R1.I4 = q3vm_argument(self, R0.I4);
/*
  R1.I4 = (2 + R0.I4) * sizeof(vmword);
  q3vm_set_word(self, R1.I4, R0);
*/
  q3vm_marshal(self, OPPARM.I4, R0);
  return 0;
}

int OP(BLOCK_COPY)  /* XXX */
{
  R2 = OPPARM;  /* Number of octets to copy */
  R1 = q3vm_drop(self);  /* From */
  R0 = q3vm_drop(self);  /* To */
#if 1 /* cute */
//crumb("Copying into [%d] from [%d] for %d octets\n", R0.I4, R1.I4, R2.I4);
  memcpy((char*)self->ram + R0.I4, (char*)self->ram + R1.I4, R2.I4);
#else /* straightforward */
  while (R2.I4 > 0)
    {
      q3vm_set_U1(self, R0.I4, q3vm_get_U1(self, R1.I4));
      R0.I4++;
      R1.I4++;
      R2.I4--;  /* counter decrement */
    }
#endif /* 0 */
  return -1;
}

int OP(SEX8)   /* Sign-extend 8-bit */
{
  R0 = q3vm_drop(self);
  if (R0.U4 & 0x80) R0.U4 |= 0xFFFFFF80;
  q3vm_push(self, R0);
  return 0;
}

int OP(SEX16)  /* Sign-extend 16-bit */
{
  R0 = q3vm_drop(self);
  if (R0.U4 & 0x8000) R0.U4 |= 0xFFFF8000;
  q3vm_push(self, R0);
  return 0;
}

/* unary operator. */
#define UNOP(vmtype, op) { R1 = q3vm_drop(self); \
  R0.vmtype = op (R1.vmtype); q3vm_push(self, R0); }

/* binary operator. */
#if 0
#define BINOP(vmtype, op) { R1 = q3vm_drop(self); R0 = q3vm_drop(self); \
 R2.vmtype = R0.vmtype op R1.vmtype; \
 crumb("Op %s with %d and %d => %d\n", #op, R0.I4, R1.I4, R2.I4); \
 q3vm_push(self, R2); return 0; }
#else
#define BINOP(vmtype, op) { R1 = q3vm_drop(self); R0 = q3vm_drop(self); \
 R2.vmtype = R0.vmtype op R1.vmtype; \
 q3vm_push(self, R2); return 0; }
#endif /* 0 */

int OP(NEGI)  /* [DP] <- -[DP] */
  UNOP(I4, -)

int OP(ADD)   /* [DP-1] <- [DP-1] + [DP]; DP <- DP-1 */
  BINOP(I4, +)

int OP(SUB)   /* [DP-1] <- [DP-1] - [DP]; DP <- DP-1 */
  BINOP(I4, -)

int OP(DIVI)   /* [DP-1] <- [DP-1] / [DP]; DP <- DP-1 */
  BINOP(I4, /)

int OP(DIVU)   /* [DP-1] <- [DP-1] / [DP]; DP <- DP-1 */
  BINOP(U4, /)

int OP(MODI)   /* [DP-1] <- [DP-1] % [DP]; DP <- DP-1 */
  BINOP(I4, %)

int OP(MODU)   /* [DP-1] <- [DP-1] % [DP]; DP <- DP-1 */
  BINOP(U4, %)

int OP(MULI)   /* [DP-1] <- [DP-1] * [DP]; DP <- DP-1 */
  BINOP(I4, *)

int OP(MULU)   /* [DP-1] <- [DP-1] * [DP]; DP <- OP-1 */
  BINOP(U4, *)

int OP(BAND)   /* [DP-1] <- [DP-1] & [DP]; DP <- DP-1 */
  BINOP(U4, &)

int OP(BOR)   /* [DP-1] <- [DP-1] | [DP]; DP <- DP-1 */
  BINOP(U4, |)

int OP(BXOR)   /* [DP-1] <- [DP-1] ^ [DP]; DP <- DP-1 */
  BINOP(U4, ^)

int OP(BCOM)   /* [DP] <- ~[DP] */
  UNOP(U4, ~)

int OP(LSH)   /* [DP-1] <- [DP-1] << [DP]; DP <- DP-1 */
  BINOP(U4, <<)

int OP(RSHI)  /* [DP-1] <- [DP-1] >> [DP]; DP <- DP-1 */
/*
  BINOP(I4, >>)
*/
{
  R1 = q3vm_drop(self);
  R0 = q3vm_drop(self);
#if 0
  while (R1.I4-- > 0)
      R0.I4 /= 2;
  R2 = R0;
#else
  R2.I4 = R0.I4 >> R1.I4;
#endif /* 0 */
  q3vm_push(self, R2);
  return 0;
}

int OP(RSHU)   /* [DP-1] <- [DP-1] >> [DP]; DP <- DP-1 */
  BINOP(U4, >>)

int OP(NEGF)   /* [DP] <- -[DP] */
  UNOP(F4, -)

int OP(ADDF)   /* [DP-1] <- [DP-1] + [DP]; DP <- DP-1 */
  BINOP(F4, +)

int OP(SUBF)   /* [DP-1] <- [DP-1] - [DP]; DP <- DP-1 */
  BINOP(F4, -)

int OP(DIVF)   /* [DP-1] <- [DP-1] / [DP]; DP <- DP-1 */
  BINOP(F4, /)

int OP(MULF)   /* [DP-1] <- [DP-1] / [DP]; DP <- DP-1 */
  BINOP(F4, *)

int OP(CVIF)   /* [DP] <- [DP] */
{
  R0 = q3vm_drop(self);
  R1.I4 = (vmI4)(R0.F4);
  q3vm_push(self, R1);
  return 0;
}

int OP(CVFI)   /* [DP] <- [DP] */
{
  R0 = q3vm_drop(self);
  R1.I4 = (vmF4)(R0.I4);
  q3vm_push(self, R1);
  return 0;
}













/*
 opcode names.
*/

#undef OP
#define OP(n) #n

const char *opnames[] = {
  OP(UNDEF),
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
};



/* Opcode lookup table. */
#undef OP
#define OP(n) q3vm_op##n

opfunc optable[] = {
  OP(UNDEF),
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
};

