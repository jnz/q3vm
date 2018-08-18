/*
Triseism - Standlone interpreter of Quake III Virtual Machine
Copyright 2003  PhaethonH <phaethon@linux.ucla.edu>

Permission granted to copy, modify, distribute, or otherwise use this code,
provided this copyright notice remains intact
*/

/*
 vm mantle: Q3VM system traps/calls (syscalls)
*/

#ifndef _Q3VMTRAP_H_
#define _Q3VMTRAP_H_

#include "q3vm.h"

/*
 VM PC -1  ==  trap 0
 VM PC -2  ==  trap 1
 VM PC -3  ==  trap 2
 VM PC -4  ==  trap 3
 ...
*/

typedef int (*q3vm_trap)(q3vm_t *);

/* thunking layer */
int q3vm_syscall (q3vm_t *self, int trap);

#endif /* _Q3VMTRAP_H_ */
