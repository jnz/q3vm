#include <stdio.h>
#include "q3vm.h"
#include "q3vmops.h"

q3vm_t vm;

int main(int argc, char **argv)
{
    const char *qvmfname;
    vmword w;

    if (argc < 2)
    {
        printf("No virtual machine supplied. Example: q3vm bytecode.qvm\n");
        return -1;
    }

    q3vm_init (&vm, 56);

    qvmfname = argv[1];
    printf("Using file: %s\n", qvmfname);

    if (!q3vm_load_name(&vm, qvmfname))
    {
        printf("Could not open file.\n");
        return 0;
    }

    //  q3vm_disasm(&vm);

    //  vm.PC = vm.romlen + 1;  /* Terminate.  Return to Termination. */
    printf("Running VM @ %d...\n", vm.PC);
    q3vm_call(&vm, 0, 0, 88, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);  /* vmMain */
    printf("vmMain finished, stack size %d\n", vm.datastack - vm.DP);
    w = q3vm_drop(&vm);
    printf("Return value = %08X\n", w.U4);

    return w.U4;
}
