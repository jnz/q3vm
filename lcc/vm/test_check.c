/* test_check.c -- deliberate errors for the VM_CHECKED build, selected by the argument
 * 1: read of an uninitialized local, 2: division by zero, 3: read of a fresh malloc block */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    int   which = argc > 1 ? argv[1][0] - '0' : 0;
    int   local;
    int   zero = 0;
    int*  block;

    if (which == 1)
    {
        fprintf(stdout, "value %d\n", local);
    }
    else if (which == 2)
    {
        fprintf(stdout, "value %d\n", 7 / zero);
    }
    else if (which == 3)
    {
        block = malloc(16);
        fprintf(stdout, "value %d\n", block[1]);
    }
    fprintf(stdout, "done\n");
    return 0;
}
