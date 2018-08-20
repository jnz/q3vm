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

/* The compiled bytecode calls native functions,
   defined in this file. */
intptr_t systemCalls(intptr_t *args);

/* Load an image from a file. Data is allocated with malloc.
   Call free() to unload image. */
uint8_t* loadImage(const char* filepath);

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("No virtual machine supplied. Example: q3vm bytecode.qvm\n");
        return -1;
    }

	char* filepath = argv[1];
	uint8_t* image = loadImage(filepath);
	if (!image)
	{
		return -1;
	}

    vm_t* vm = VM_Create(filepath, image, systemCalls);
    free(image); /* we can release the memory now */

    VM_Call(vm, 0);
	VM_Free(vm);

    return 0;
}

void Com_Error(int level, const char *error)
{
    fprintf(stderr, "Err: %s\n", error);
    exit(level);
}

uint8_t* loadImage(const char* filepath)
{
	FILE* f; /* bytecode input file */
	uint8_t* image = NULL; /* bytecode buffer */
	size_t sz; /* bytecode file size */

    f = fopen(filepath, "rb");
    if (!f)
    {
        fprintf(stderr, "Failed to open file %s.\n", filepath);
		return NULL;
    }
    /* calculate file size */
    fseek(f, 0L, SEEK_END);
    sz = ftell(f);
    rewind(f);

    image = (uint8_t*)malloc(sz);
    if (!image)
    {
        fclose(f);
		return NULL;
    }

    if (fread(image, 1, sz, f) != sz)
    {
        free(image);
        fclose(f);
		return NULL;
    }

	return image;
}

intptr_t systemCalls( intptr_t *args )
{
    switch (args[0])
	{
    case 0: /* PRINTF */
        printf("%s", (const char*)VMA(1));
        return 0;
    case 1: /* ERROR */
        fprintf(stderr, "%s", (const char*)VMA(1));
        return 0;

    case 2: /* MEMSET */
        memset(VMA(1), args[2], args[3]);
        return 0;

    case 3: /* MEMCPY */
        memcpy(VMA(1), VMA(2), args[3]);
        return 0;

    default:
        fprintf(stderr, "Bad game system trap: %ld", (long int) args[0] );
    }
    return 0;
}
