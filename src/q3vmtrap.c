/*
Triseism - Standlone interpreter of Quake III Virtual Machine
Copyright 2003  PhaethonH <phaethon@linux.ucla.edu>

Permission granted to copy, modify, distribute, or otherwise use this code,
provided this copyright notice remains intact
*/

/*
  vm mantle: Q3VM system traps (syscalls)
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "q3vmtrap.h"


#define TRAP(name) trap_##name
#define PARM (q3vm_t *self)

#define ARG(n) (q3vm_get_I4(self, self->RP + ((2 + n) * sizeof(vmword))))

TRAP(Print) PARM
{
  char *text;

//crumb("SYSCALL Print [%d]\n", ARG(0)); 
  text = (char*)(self->ram) + ARG(0);
//crumb("PRINTING [%s]\n", text);
  printf("%s", text);
  return 0;
}


TRAP(Error) PARM
{
  char *msg;

  msg = (char*)(self->ram) + ARG(0);
  printf("%s", msg);
  self->PC = self->romlen + 1;
  return 0;
}

/*
 speed-purposes syscalls.
*/

/* void *memset (void *dest, int c, size_t count) */
TRAP(memset) PARM
{
  // int arg[3];
  char *dest;
  int c;
  size_t count;

  dest = (char*)(self->ram) + ARG(0);
  c = ARG(1);
  count = ARG(2);
  memset(dest, c, count);
  return 0;
}


/* void *memcpy (void *dest, const void *src, size_t count) */
TRAP(memcpy) PARM
{
  // int arg[3];
  char *dest;
  char *src;
  size_t count;

  dest = (char*)(self->ram) + ARG(0);
  src = (char*)(self->ram) + ARG(1);
  count = ARG(2);
  memcpy(dest, src, count);
  return 0;
}


/* char *strncpy (char *strDest, const char *strSource, size_t count) */
TRAP(strncpy) PARM
{
  char *dst, *src;
  int count;
  int retval;

//crumb("SYSCALL strncpy [%d] <- [%d] + %d\n", ARG(0), ARG(1), ARG(2));
  dst = (char*)(self->ram) + ARG(0);
  src = (char*)(self->ram) + ARG(1);
  count = ARG(2);
  strncpy(dst, src, count);
  retval = dst - (char*)self->ram;
  return retval;
}

int
q3vm_syscall (q3vm_t *self, int trap)
{
  int retval;
  vmword w;

  retval = 0;
  switch (trap)
    {
#define MAPTRAP(n, f) case n: retval = trap_##f (self); break;
      MAPTRAP(-1, Print)
      MAPTRAP(-2, Error)
      MAPTRAP(-3, memset)
      MAPTRAP(-4, memcpy)
      MAPTRAP(-5, strncpy)
    }

  w = q3vm_drop(self);
  self->PC = w.I4;
  w.I4 = retval;
  q3vm_push(self, w);
  return 1;
}

