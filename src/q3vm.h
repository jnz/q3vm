/*
Triseism - Standlone interpreter of Quake III Virtual Machine
Copyright 2003  PhaethonH <phaethon@linux.ucla.edu>

Permission granted to copy, modify, distribute, or otherwise use this code,
provided this copyright notice remains intact
*/

/*
 vm crust: Types and public interface for VM.
*/

#ifndef _Q3VM_H_
#define _Q3VM_H_

/*
Q3VM has four segments: code, data, lit, bss
 code - stored program
 data - initialized allocated space of word (32-bit) size (constants, arrays)
 lit - initialized allocated space of octet (8-bit) size (strings)
 bss - uninitialized storage (declared variables)

$code is processed into 32-bit pairs.  First half is the bytecode itself, second half is any code-space parameter (peephole?), which is zero and ignored if not needed.
$data is loaded with optional byte-swapping for endianness in VM RAM.
$lit follows directly after $data in the VM RAM.
$bss is a large swatch of zeroes following in the VM RAM.
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#define crumb printf

#define STACKSIZE  0x10000

/* VM data types represented in host memory. */
typedef int32_t vmI4;   /* signed 32-bit integer. */
typedef uint32_t vmU4;  /* unsigned 32-bit integer. */
typedef int16_t vmI2;   /* signed 16-bit integer. */
typedef uint16_t vmU2;  /* unsigned 16-bit integer. */
typedef int8_t vmI1;    /* signed 8-bit integer. */
typedef uint8_t vmU1;   /* unsigned 8-bit integer. */
typedef float vmF4;     /* 32-bit floating-point */

/* A word in VM memory. */
union vmword {
  vmI4 I4;
  vmU4 U4;
  vmF4 F4;
  vmI2 I2[2];
  vmU2 U2[2];
  vmI1 I1[4];
  vmU1 U1[4];
};

typedef union vmword vmword;



/* Terminate q3vm_run() after this many cycles (avoid infinite loop). */
#define WATCHDOG_DEFAULT 1000000  /* 1M cycles */


/* One VM instruction (processed-on-load). */
struct q3vm_rom_t {
  int opcode;
  vmword parm;
//  opfunc opfunc;
};

typedef struct q3vm_rom_t q3vm_rom_t;


/* Byte-oriented RAM. */
typedef vmU1 q3vm_ram_t;


struct q3vm_t {
  int bigendian;  /* host is big-endian (requires byte-swapping). */

/* Memory spaces. */
  char *hunk;    /* hunk space (malloc'd). */
  int hunksize;  /* total hunk size. */
  int freehunk;  /* free pointer. */
  int romlen;    /* Read-Only Memory (code). */
  q3vm_rom_t *rom;
  int ramlen;    /* Random-Access Memory (data). */
  q3vm_ram_t *ram;

  int datastack; /* data stack. */
  int retstack;  /* return stack. */

  vmword r[4];   /* AU registers. */
  int DP;        /* Datastack pointer. */
  int RP;        /* Return stack pointer. */
  int PC;        /* Program Counter. */
//  int AP;        /* Argument pointer.  (hrm...) */

  /* various flags. */
  int cm:1;

  int watchdog;  /* maximum execution time (Halting Problem). */
};

typedef struct q3vm_t q3vm_t;



/* For loading from .qvm file */
enum {
  QVM_MAGIC = 0x12721444,
};

struct qvm_header_t {
  int magic;
  /* not-entirely-RISC ISA, so instruction count != codelen */
  int inscount; /* instruction count. */
  int codeoff;  /* file offset of code segment. */
  int codelen;  /* length of code segment, in octets. */
  int dataoff;  /* file offset of data segment. */
  int datalen;  /* length of data segment, in octets. */
  int litlen;   /* length of lit segment (which is embedded in data segment). */
  int bsslen;   /* length of bss segment. */
};

typedef struct qvm_header_t qvm_header_t;



/*
Public interface
*/

#undef _THIS
#define _THIS q3vm_t *self

q3vm_t *q3vm_init (_THIS, int hunkmegs);
int q3vm_load_name (_THIS, const char *fname);
int q3vm_load_file (_THIS, FILE *qvmfile);
int q3vm_load_fd (_THIS, int fd);
void q3vm_destroy (_THIS);
int q3vm_run (_THIS);       /* Run until machine halts. */

/* Stack ops */  /* based on Forth */
 /* data stack */
vmword q3vm_fetch (_THIS);
vmword q3vm_drop (_THIS);
vmword q3vm_push (_THIS, vmword val);
vmword q3vm_dup (_THIS);
vmword q3vm_nip (_THIS);
 /* return stack */
vmword q3vm_to_r (_THIS);     /* >r */
vmword q3vm_r_from (_THIS);   /* r> */
vmword q3vm_rfetch (_THIS);   /* r@ */
vmword q3vm_rdrop (_THIS);    /* rdrop */
vmword q3vm_rpush (_THIS, vmword val);

/* Execution ops */
/* C wrapper */
int q3vm_call (_THIS, int addr, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11, int arg12);  /* Make a call to a VM procedure. */
int q3vm_marshal (_THIS, int addr, vmword arg);  /* Marshal an argument for an upcoming procedure call */

int q3vm_opcode_paramp (int opcode);  /* If opcode wants parameter */

vmword q3vm_get_word (_THIS, int addr);
vmword q3vm_set_word (_THIS, int addr, vmword val);
vmI4 q3vm_get_I4 (_THIS, int addr);
vmI4 q3vm_set_I4 (_THIS, int addr, vmI4 val);
vmU4 q3vm_get_U4 (_THIS, int addr);
vmU4 q3vm_set_U4 (_THIS, int addr, vmU4 val);
vmF4 q3vm_get_F4 (_THIS, int addr);
vmF4 q3vm_set_F4 (_THIS, int addr, vmF4 val);
vmI2 q3vm_get_I2 (_THIS, int addr);
vmI2 q3vm_set_I2 (_THIS, int addr, vmI2 val);
vmU2 q3vm_get_U2 (_THIS, int addr);
vmU2 q3vm_set_U2 (_THIS, int addr, vmU2 val);
vmI1 q3vm_get_I1 (_THIS, int addr);
vmI1 q3vm_set_I1 (_THIS, int addr, vmI1 val);
vmU1 q3vm_get_U1 (_THIS, int addr);
vmU1 q3vm_set_U1 (_THIS, int addr, vmU1 val);

#define GET(type, seg, off) q3vm_get_##type (self, seg, off)
#define SET(type, seg, off, val) q3vm_set_##type (self, seg, off, val)


#endif /* _Q3VM_H_ */
