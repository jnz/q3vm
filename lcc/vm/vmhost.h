/* vmhost.h -- the host side of the VM: system calls, files and the runner */

#ifndef VMHOST_H
#define VMHOST_H

#include <stdint.h>

#include "vm.h"

intptr_t vmhost_syscalls(vm_t* vm, intptr_t* args);
void     vmhost_init(int argc, char** argv);
int      vmhost_run(const uint8_t* image, int size, int argc, char** argv);
/* the option -heap=SIZE (K and M suffixes): the heap of the VM images, which the
 * host adds to the bss of each image run. Returns 1 for the option, 0 for another
 * argument and -1 for a bad value. */
int      vmhost_option(const char* arg);

#endif
