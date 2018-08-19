/*
   Triseism - Standlone interpreter of Quake III Virtual Machine
   Copyright 2003  PhaethonH <phaethon@linux.ucla.edu>

   Permission granted to copy, modify, distribute, or otherwise use this code,
   provided this copyright notice remains intact
   */

/*
   vm crust: public interface for VM
   */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "q3vm.h"

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

static const char *opnames[OP(NUM_OPS)];
typedef int (*opfunc)(struct q3vm_t *self, vmword parm);
static const opfunc optable[OP(NUM_OPS)];

const char *SEGMENT_CODE = "code";
const char *SEGMENT_DATA = "data";
const char *SEGMENT_LIT = "lit";
const char *SEGMENT_BSS = "bss";

/* Stack ops */  /* based on Forth */
 /* data stack */
static vmword q3vm_fetch (_THIS);
static vmword q3vm_dup (_THIS);
static vmword q3vm_nip (_THIS);
 /* return stack */
static vmword q3vm_to_r (_THIS);     /* >r */
static vmword q3vm_r_from (_THIS);   /* r> */
static vmword q3vm_rfetch (_THIS);   /* r@ */
static vmword q3vm_rdrop (_THIS);    /* rdrop */
static vmword q3vm_rpush (_THIS, vmword val);

static vmword q3vm_get_word (_THIS, int addr);
static vmword q3vm_set_word (_THIS, int addr, vmword val);
static vmI4 q3vm_set_I4 (_THIS, int addr, vmI4 val);
static vmU4 q3vm_get_U4 (_THIS, int addr);
static vmU4 q3vm_set_U4 (_THIS, int addr, vmU4 val);
static vmF4 q3vm_get_F4 (_THIS, int addr);
static vmF4 q3vm_set_F4 (_THIS, int addr, vmF4 val);
static vmI2 q3vm_get_I2 (_THIS, int addr);
static vmI2 q3vm_set_I2 (_THIS, int addr, vmI2 val);
static vmU2 q3vm_get_U2 (_THIS, int addr);
static vmU2 q3vm_set_U2 (_THIS, int addr, vmU2 val);
static vmI1 q3vm_get_I1 (_THIS, int addr);
static vmI1 q3vm_set_I1 (_THIS, int addr, vmI1 val);
static vmU1 q3vm_get_U1 (_THIS, int addr);
static vmU1 q3vm_set_U1 (_THIS, int addr, vmU1 val);


/* Read one octet from file. */
int q3vm_read_octet (FILE *qvmfile)
{
    int o;
    o = fgetc(qvmfile);
    if (o < 0) o = 0;  /* EOF (hack) */
    return o;
}

/* Read little-endian 32-bit integer from file. */
int q3vm_read_int (FILE *qvmfile)
{
    int a, b, c, d, n;

    a = q3vm_read_octet(qvmfile);
    b = q3vm_read_octet(qvmfile);
    c = q3vm_read_octet(qvmfile);
    d = q3vm_read_octet(qvmfile);
    n = (a) | (b << 8) | (c << 16) | (d << 24);
    return n;
}


#undef _THIS
#define _THIS q3vm_t *self

int q3vm_error (const char *fmt, ...)
{
    va_list vp;
    int n;

    va_start(vp, fmt);
    n = vfprintf(stderr, fmt, vp);
    va_end(vp);
    return n;
}

q3vm_t* q3vm_init (_THIS, int hunkmegs)
{
    if (!self)
    {
        self = (q3vm_t*)malloc(sizeof(q3vm_t));
        if (!self) return NULL;
    }
    crumb("Initializing VM\n");
    memset(self, 0, sizeof(*self));
    self->hunksize = hunkmegs * 1048576;
    crumb("Allocating hunk %d bytes\n", self->hunksize);
    self->hunk = calloc(self->hunksize, sizeof(self->hunk[0]));
    crumb("VM initialized.\n");
    return self;
}


void q3vm_destroy (_THIS)
{
    if (self->hunk)
        free(self->hunk);
    self->hunk = NULL;
    memset(self, 0, sizeof(*self));
}


/* Returns number of bytes of parameter (from code segment) wanted by opcode. */
int q3vm_opcode_paramp (int opcode)
{
    switch (opcode)
    {
        case OP(ENTER):
        case OP(LEAVE):
        case OP(LOCAL):
        case OP(EQ):
        case OP(NE):
        case OP(LTI):
        case OP(LEI):
        case OP(GTI):
        case OP(GEI):
        case OP(LTU):
        case OP(LEU):
        case OP(GTU):
        case OP(GEU):
        case OP(EQF):
        case OP(NEF):
        case OP(LTF):
        case OP(LEF):
        case OP(GTF):
        case OP(GEF):
        case OP(CONST):
        case OP(BLOCK_COPY):
            return sizeof(vmU4);
            break;
        case OP(ARG):
            return sizeof(vmU1);
            break;
    }
    return 0;
}


static inline int q3vm_step (_THIS)
{
    int codenum;
    vmword parameter;
    opfunc f;

    if (self->PC > self->romlen)
    {
        return 0;
    }
    if (self->PC < 0)
    {
        /* system trap. */
        return q3vm_syscall(self, self->PC);
    }
    codenum = self->rom[self->PC].opcode;
    parameter = self->rom[self->PC].parm;
    //crumb("Dispatching opcode %d[%s] (PC=%d, DP=%d RP=%d)\n", codenum, opnames[codenum], self->PC, self->DP, self->RP);
    self->PC++;
    f = optable[codenum];
    f(self, parameter);
    return 1;
}


int q3vm_run (_THIS)
{
    int n;

    n = 1;
    while (n)
    {
        n = q3vm_step(self);
    }
    return 1;
}

int q3vm_load_file (_THIS, FILE *qvmfile)
{
    qvm_header_t qvminfo;
    int i, n;
    vmU1 x[4];
    vmword w;

    crumb("Loading file...\n");
    qvminfo.magic = q3vm_read_int(qvmfile); /* magic. */
    if (qvminfo.magic != QVM_MAGIC)
    {
        q3vm_error("Does not appear to be a QVM file.");
        /* XXX: option to force continue. */
        return 0;
    }
    crumb("Magic OK\n");
    /* variable-length instructions mean instruction count != code length */
    qvminfo.inscount = q3vm_read_int(qvmfile);
    qvminfo.codeoff = q3vm_read_int(qvmfile);
    qvminfo.codelen = q3vm_read_int(qvmfile);
    qvminfo.dataoff = q3vm_read_int(qvmfile);
    qvminfo.datalen = q3vm_read_int(qvmfile);
    qvminfo.litlen = q3vm_read_int(qvmfile);
    qvminfo.bsslen = q3vm_read_int(qvmfile);

    /* Code segment should follow... */
    /* XXX: use fseek with SEEK_CUR? */
    crumb("Searching for .code @ %d from %u\n",
          qvminfo.codeoff, (unsigned)ftell(qvmfile));
    //  self->rom = (q3vm_rom_t*)(self->hunk);  /* ROM-in-hunk */
    self->rom = (q3vm_rom_t*)calloc(qvminfo.inscount, sizeof(self->rom[0]));
    while (ftell(qvmfile) < qvminfo.codeoff)
        q3vm_read_octet(qvmfile);
    while (self->romlen < qvminfo.inscount)
    {
        n = q3vm_read_octet(qvmfile);
        w.I4 = 0;
        if ((i = q3vm_opcode_paramp(n)))
        {
            x[0] = x[1] = x[2] = x[3] = 0;
            fread(&x, 1, i, qvmfile);
            w.U4 = (x[0]) | (x[1] << 8) | (x[2] << 16) | (x[3] << 24);
        }
        self->rom[self->romlen].opcode = n;
        self->rom[self->romlen].parm = w;
        self->romlen++;
    }
    crumb("After loading code: at %u, should be %d\n", (unsigned)ftell(qvmfile), qvminfo.codeoff + qvminfo.codelen);

    /* Then data segment. */
    //  self->ram = self->hunk + ((self->romlen + 3) & ~3);  /* RAM-in-hunk */
    self->ram = self->hunk;
    crumb("Searching for .data @ %d from %d\n",
          qvminfo.dataoff, (unsigned)ftell(qvmfile));
    while (ftell(qvmfile) < qvminfo.dataoff)
    {
        q3vm_read_octet(qvmfile);
    }
    for (n = 0; n < (qvminfo.datalen / sizeof(vmU1)); n++)
    {
        i = fread(&x, 1, sizeof(x), qvmfile);
        w.U4 = (x[0]) | (x[1] << 8) | (x[2] << 16) | (x[3] << 24);
        *((vmword*)(self->ram + self->ramlen)) = w;
        self->ramlen += sizeof(vmword);
    }
    /* lit segment follows data segment. */
    /* Assembler should have already padded properly. */
    crumb("Loading .lit\n");
    for (n = 0; n < (qvminfo.litlen / sizeof(vmU1)); n++)
    {
        i = fread(&x, 1, sizeof(x), qvmfile);
        memcpy(&(w.U1), &x, sizeof(x));  /* no byte-swapping. */
        *((vmword*)(self->ram + self->ramlen)) = w;
        self->ramlen += sizeof(vmword);
    }
    /* bss segment. */
    crumb("Allocating .bss %d (%X) bytes\n", qvminfo.bsslen, qvminfo.bsslen);
    /* huge empty chunk. */
    self->ramlen += qvminfo.bsslen;

    self->freehunk = self->hunksize - ((self->ramlen * sizeof(vmU1)) + 4);

    crumb("VM hunk has %d of %d bytes free (RAM = %d B).\n", self->freehunk, self->hunksize, self->ramlen);
    if (self->ramlen > self->hunksize)
    {
        q3vm_error("ERROR: Hunk exhausted.\n");
        return 0;
    }

    /* set up stack. */
    {
        int stacksize = STACKSIZE;
        self->retstack = self->ramlen;
        self->datastack = self->ramlen - (stacksize / 2);
        self->RP = self->retstack;
        self->DP = self->datastack;
    }

    /* set up PC for return-to-termination. */
    self->PC = self->romlen + 1;

    return 1;
}

int q3vm_load_name (_THIS, const char *fname)
{
    FILE *qvmfile;

    if (!(qvmfile = fopen(fname, "rb")))
    {
        return 0;
    }
    return q3vm_load_file(self, qvmfile);
}

/* Stack operations. */
static vmword q3vm_fetch (_THIS)
{
    vmword w;
    w.I4 = 0;
    //  crumb("Fetching from [%d]\n", self->DP);
    if (self->DP < 0)
        return w;
    w = q3vm_get_word(self, self->DP);
    return w;
}

vmword q3vm_drop (_THIS)
{
    vmword w;

    w = q3vm_fetch(self);
    //crumb("Data pop [%d] %d\n", self->DP, w.I4);
    self->DP += sizeof(w);
    return w;
}

vmword q3vm_push (_THIS, vmword val)
{
    self->DP -= sizeof(val);
    q3vm_set_word(self, self->DP, val);
    //crumb("Data push [%d] %d\n", self->DP, val.I4);
    return val;
}

static vmword q3vm_dup (_THIS)  /* w -- w w */
{
    vmword w;

    w = q3vm_fetch(self);
    q3vm_push(self, w);
    return w;
}

static vmword q3vm_nip (_THIS)
{
    vmword w;

    w = q3vm_drop(self);
    q3vm_drop(self);
    q3vm_push(self, w);
    return w;
}

static vmword q3vm_to_r (_THIS)
{
    vmword w;

    w = q3vm_drop(self);
    q3vm_rpush(self, w);
    return w;
}

static vmword q3vm_r_from (_THIS)
{
    vmword w;

    w = q3vm_rdrop(self);
    q3vm_push(self, w);
    return w;
}

static vmword q3vm_r_fetch (_THIS)
{
    vmword w;
    w.I4 = 0;
    if (self->RP < 0)
        return w;
    w = q3vm_get_word(self, self->RP);
    return w;
}

static vmword q3vm_rdrop (_THIS)
{
    vmword w;

    w = q3vm_r_fetch(self);
    self->RP += sizeof(vmword);
    return w;
}

static vmword q3vm_rpush (_THIS, vmword val)
{
    self->RP -= sizeof(val);
    q3vm_set_word(self, self->RP, val);
    //  crumb("Retstack push [%d] %d\n", self->RP, val.I4);
    return val;
}

int q3vm_marshal (_THIS, int addr, vmword arg)
{
#if 0
    vmword w;
    w = q3vm_get_word(self, self->RP + sizeof(vmword));  /* cur arg off */
    crumb("Marshaling argument %08X into [%d]\n", arg.I4, self->RP + 8 + w.I4);
    q3vm_set_word(self, self->RP + (2 * sizeof(vmword)) + w.I4, arg);
    w.I4 += sizeof(vmword);
    q3vm_set_word(self, self->RP + sizeof(vmword), w); /* cur arg off */
    return w.I4;
#else /* 0 */
    int loc = self->RP + addr;
    //crumb("Marshalling %d as arg %d [%d]\n", arg.I4, (addr / sizeof(vmword)), loc);
    q3vm_set_word(self, loc, arg);
    return loc;
#endif /* 0 */
}

/* Absolute address of `localnum'th local. */
static int q3vm_local (_THIS, int localnum)
{
    int localoff;

    localoff = localnum * sizeof(vmword);
    //  localoff += 2 * sizeof(vmword);
    localoff = self->RP + localoff;
    return localoff;
}

int q3vm_call (_THIS, int addr, int arg0, int arg1, int arg2, int arg3, int
               arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int
               arg10, int arg11, int arg12)
{
    vmword w;
    int i;

    /* Set up call. */
    q3vm_opPUSH(self);
    crumb("Starting with PC=%d, DP=%d, RP=%d to %d\n", self->PC, self->DP, self->RP, addr);
    w.I4 = (12 + 2) * sizeof(vmword);
    q3vm_opENTER(self, w);
    i = 8;
    w.I4 = arg0; q3vm_marshal(self, i, w); i += 4;
    w.I4 = arg1; q3vm_marshal(self, i, w); i += 4;
    w.I4 = arg2; q3vm_marshal(self, i, w); i += 4;
    w.I4 = arg3; q3vm_marshal(self, i, w); i += 4;
    w.I4 = arg4; q3vm_marshal(self, i, w); i += 4;
    w.I4 = arg5; q3vm_marshal(self, i, w); i += 4;
    w.I4 = arg6; q3vm_marshal(self, i, w); i += 4;
    w.I4 = arg7; q3vm_marshal(self, i, w); i += 4;
    w.I4 = arg8; q3vm_marshal(self, i, w); i += 4;
    w.I4 = arg9; q3vm_marshal(self, i, w); i += 4;
    w.I4 = arg10; q3vm_marshal(self, i, w); i += 4;
    w.I4 = arg11; q3vm_marshal(self, i, w); i += 4;
    w.I4 = arg12; q3vm_marshal(self, i, w); i += 4;
    w.I4 = addr;  /* calling this */
    q3vm_push(self, w);
    q3vm_opCALL(self, w);
    printf("Upon running PC=%d, DP=%d, RP=%d\n", self->PC, self->DP, self->RP);
    q3vm_run(self);
    printf("At finish PC=%d, DP=%d, RP=%d\n", self->PC, self->DP, self->RP);
    q3vm_opLEAVE(self, (12+2) * sizeof(vmword));
    return 0;
}



/******************/
/* Memory access. */
/******************/

static vmword q3vm_get_word (_THIS, int addr)
{
    vmword w;
    // vmU1 x[4];

    if (addr < 0) addr = 0;
    if (addr > self->ramlen - 4) addr = self->ramlen - 4;
    w = *((vmword*)(self->ram + addr));
    return w;
}

static vmword q3vm_set_word (_THIS, int addr, vmword val)
{
    if (addr < 0) addr = 0;
    if (addr > self->ramlen - 4) addr = self->ramlen - 4;
    *((vmword*)(self->ram + addr)) = val;
    //crumb("Setting word [%d] = %d\n", addr, val.I4);
    return val;
}

vmI4 q3vm_get_I4 (_THIS, int addr)
{
    vmI4 retval;
    vmword w;

    w = q3vm_get_word(self, addr);
    retval = w.I4;
    return retval;
}

static vmI4 q3vm_set_I4 (_THIS, int addr, vmI4 val)
{
    vmword w;

    w.I4 = val;
    q3vm_set_word(self, addr, w);
    return val;
}

static vmU4 q3vm_get_U4 (_THIS, int addr)
{
    vmword w;

    w = q3vm_get_word(self, addr);
    return w.U4;
}

static vmU4 q3vm_set_U4 (_THIS, int addr, vmU4 val)
{
    vmword w;

    w.U4 = val;
    q3vm_set_word(self, addr, w);
    return val;
}

static vmF4 q3vm_get_F4 (_THIS, int addr)
{
    vmword w;

    w = q3vm_get_word(self, addr);
    return w.F4;
}

static vmF4 q3vm_set_F4 (_THIS, int addr, vmF4 val)
{
    vmword w;

    w.F4 = val;
    q3vm_set_word(self, addr, w);
    return val;
}

static vmI2 q3vm_get_I2 (_THIS, int addr)
{
    vmword w;

    w = q3vm_get_word(self, addr);
    return w.I2[0];
}

static vmI2 q3vm_set_I2 (_THIS, int addr, vmI2 val)
{
    vmword w;

    w = q3vm_get_word(self, addr);
    w.I2[0] = val;
    q3vm_set_word(self, addr, w);
    return w.I2[0];
}

static vmU2 q3vm_get_U2 (_THIS, int addr)
{
    vmword w;

    w = q3vm_get_word(self, addr);
    return w.U2[0];
}

static vmU2 q3vm_set_U2 (_THIS, int addr, vmU2 val)
{
    vmword w;

    w = q3vm_get_word(self, addr);
    w.U2[0] = val;
    q3vm_set_word(self, addr, w);
    return w.U2[0];
}

static vmI1 q3vm_get_I1 (_THIS, int addr)
{
    vmword w;

    w = q3vm_get_word(self, addr);
    return w.I1[0];
}

static vmI1 q3vm_set_I1 (_THIS, int addr, vmI1 val)
{
    vmword w;

    w = q3vm_get_word(self, addr);
    w.I1[0] = val;
    q3vm_set_word(self, addr, w);
    return w.I1[0];
}

static vmU1 q3vm_get_U1 (_THIS, int addr)
{
    vmword w;

#if 1
    w = q3vm_get_word(self, addr);
#else
    w.U1[0] = *((char*)(self->ram) + addr);
#endif /* 0 */
    return w.U1[0];
}

static vmU1 q3vm_set_U1 (_THIS, int addr, vmU1 val)
{
    vmword w;

#if 1
    w = q3vm_get_word(self, addr);
    w.U1[0] = val;
    q3vm_set_word(self, addr, w);
#else
    w.U1[0] = val;
    *((char*)(self->ram) + addr) = w.U1[0];
#endif /* 0 */
    return w.U1[0];
}


/* Check that loading makes sense. */
int q3vm_disasm (_THIS)
{
    int i;
    int n;
    vmword w;
    const char *opname;

    printf("Disassembling %d instructions\n", self->romlen);
    for (i = 0; i < self->romlen; i++)
    {
        n = self->rom[i].opcode;
        w = self->rom[i].parm;
        if ((0 < n) && (n < OP(NUM_OPS)))
            opname = opnames[n];
        else
            opname = "(INVALID OPCODE)";
        if (q3vm_opcode_paramp(n))
            printf("%9d. %02X %08X %s %d\n", i, n, w.I4, opname, w.I4);
        else
            printf("%9d. %02X          %s\n", i, n, opname);
    }
    return 0;
}

#undef _THIS
#define _THIS q3vm_t *self

#undef OP
#define OP(n) q3vm_op##n (_THIS, vmword parm)

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
  // vmword w;

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

static const char *opnames[] = {
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

static const opfunc optable[] = {
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

