/* vmhost.h -- the host side of the VM: system calls, files and the runner */

#ifndef VMHOST_H
#define VMHOST_H

#include <stdint.h>

#include "vm.h"

intptr_t vmhost_syscalls(vm_t* vm, intptr_t* args);
void     vmhost_init(int argc, char** argv);
int      vmhost_run(const uint8_t* image, int size, int argc, char** argv);

#endif
