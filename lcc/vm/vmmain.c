/* vmmain.c -- entry point of the VM image. Must be the first file given to q3asm,
 * because the VM starts at the first function of the image. */

#include <stdlib.h>
#include "vmsys.h"

#define VM_ARG_MAX 512

int main(int argc, char** argv);

int vmMain(void)
{
    int    argc = vm_argc();
    char** argv = malloc(argc * sizeof(char*));
    int    i;

    for (i = 0; i < argc; i++)
    {
        argv[i] = malloc(VM_ARG_MAX);
        vm_argv(i, argv[i], VM_ARG_MAX);
    }
    return main(argc, argv);
}
