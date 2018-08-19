/*
      ___   _______     ____  __
     / _ \ |___ /\ \   / /  \/  |
    | | | |  |_ \ \ \ / /| |\/| |
    | |_| |____) | \ V / | |  | |
     \__\_______/   \_/  |_|  |_|


   Quake III Arena Virtual Machine
*/

#include <stdio.h>
#include "vm.h"

intptr_t systemCalls(intptr_t *args);

int main(int argc, char **argv)
{
    uint8_t* imageTemp; /* load bytecode here */
    FILE* f;
    char* filename;

    if (argc < 2)
    {
        printf("No virtual machine supplied. Example: q3vm bytecode.qvm\n");
        return -1;
    }

    filename = argv[1];
    f = fopen(filename, "rb");
    if (!f)
    {
        fprintf(stderr, "Failed to open file %s.\n", filename);
        return -1;
    }
    /* sz =  file size */
    fseek(f, 0L, SEEK_END);
    size_t sz = ftell(f);
    rewind(f);

    imageTemp = (uint8_t*)malloc(sz);
    if (!imageTemp)
    {
        fclose(f);
        return -1;
    }

    size_t result = fread(imageTemp, 1, sz, f);
    if (result != sz)
    {
        free(imageTemp);
        fclose(f);
        return -1;
    }
    fclose(f);

    vm_t* vm = VM_Create(filename, imageTemp, systemCalls);
    free(imageTemp); /* we can release the memory now */

    VM_Call(vm, 0);
    VM_Clear();

    return 0;
}

intptr_t systemCalls( intptr_t *args )
{
    switch( args[0] ) {
    case 0: /* PRINTF */
        Com_Printf( "%s", (const char*)VMA(1) );
        return 0;
    case 1: /* ERROR */
        Com_Error( ERR_DROP, "%s", (const char*)VMA(1) );
        return 0;

    case 2: /* MEMSET */
        Com_Memset( VMA(1), args[2], args[3] );
        return 0;

    case 3: /* MEMCPY */
        Com_Memcpy( VMA(1), VMA(2), args[3] );
        return 0;

    default:
        Com_Error( ERR_DROP, "Bad game system trap: %ld", (long int) args[0] );
    }
    return 0;
}
