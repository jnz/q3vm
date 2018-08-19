#include <stdio.h>
#include "vm_local.h"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("No virtual machine supplied. Example: q3vm bytecode.qvm\n");
        return -1;
    }

    vm_t* vm = VM_Create(argv[1], VMI_BYTECODE);
    VM_Call(vm, 0);

    return 0;
}

