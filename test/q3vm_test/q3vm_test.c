/*
      ___   _______     ____  __
     / _ \ |___ /\ \   / /  \/  |
    | | | |  |_ \ \ \ / /| |\/| |
    | |_| |____) | \ V / | |  | |
     \__\_______/   \_/  |_|  |_|


   Quake III Arena Virtual Machine
*/

#include "vm.h"
#include <stdio.h>
#include <stdlib.h>

static int g_mallocFail = -1; /* if this is not -1, malloc will fail */

/* The compiled bytecode calls native functions,
   defined in this file. */
intptr_t systemCalls(vm_t* vm, intptr_t* args);

/* Load an image from a file. Data is allocated with malloc.
   Call free() to unload image. */
uint8_t* loadImage(const char* filepath, int* size);

int testInject(const char* filepath, int offset, int opcode)
{
    vm_t     vm;
    int      imageSize;
    uint8_t* image  = loadImage(filepath, &imageSize);
    int      retVal = -1;

    if (!image)
    {
        fprintf(stderr, "Failed to load bytecode image from %s\n", filepath);
        return retVal;
    }

    fprintf(stderr, "Injecting wrong OP code %s at %i: %i\n", filepath, offset,
            opcode);
    memcpy(&image[offset], &opcode, sizeof(opcode)); /* INJECT */
    retVal = VM_Create(&vm, filepath, image, imageSize, systemCalls);
    VM_Free(&vm);
    free(image);

    return (retVal == -1) ? 0 : -1;
}

int testNominal(const char* filepath)
{
    vm_t     vm;
    int      imageSize;
    uint8_t* image  = loadImage(filepath, &imageSize);
    int      retVal = -1;

    if (!image)
    {
        fprintf(stderr, "Failed to load bytecode image from %s\n", filepath);
        return retVal;
    }

    VM_Debug(1);
    if (VM_Create(&vm, filepath, image, imageSize, systemCalls) == 0)
    {
        /* normal call, should give us 0 */
        retVal = VM_Call(&vm, 0);
        /* now do the proper call, this should give us 333 */
        retVal += VM_Call(&vm, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
        /* so now retVal should be 333 if everything is as expected */
        printf("Result (should be 333): %i\n", retVal);
        /* now do an invalid function call within the VM */
        // FIXME reenable this:
        // retVal += (VM_Call(&vm, 2)+1); /* we expect a -1, so we add a +1 to cancel it out */
    }
    VM_VmProfile_f(&vm);
    VM_Free(&vm);
    free(image);

    if (retVal == 333)
    {
        printf("All tests passed [x]\n");
        return 0;
    }
    else
    {
        printf("Test suite failed!!! [ ]\n");
        return -1;
    }
}

void testArguments(void)
{
    vm_t vm = { 0 };
    int  imageSize;

    vm.codeLength = 0;
    VM_Call(&vm, 0);

    VM_ArgPtr(0, NULL);
    VM_ArgPtr(1, NULL);
    VM_MemoryRangeValid(0, 0, NULL);
    loadImage(NULL, &imageSize);
    loadImage("invalidpathfoobar", &imageSize);
    VM_Create(NULL, NULL, NULL, 0, NULL);
    VM_Create(&vm, NULL, NULL, 0, NULL);
    VM_Create(&vm, NULL, NULL, 0, systemCalls);
    VM_Create(&vm, "test", NULL, 0, systemCalls);

    uint8_t bogus[] = "bogusbogusbogubogusbogus"
                      "bogusbogusbogubogusbogus"
                      "bogusbogusbogubogusbogus"
                      "bogusbogusbogubogusbogus";
    VM_Create(&vm, "test", bogus, sizeof(bogus), NULL);
    VM_Create(&vm, "test", bogus, sizeof(bogus), systemCalls);
    VM_Create(&vm, "test", bogus, sizeof(vmHeader_t) - 4, systemCalls);

    vmHeader_t vmHeader       = { 0 };
    vmHeader.vmMagic          = VM_MAGIC;
    vmHeader.instructionCount = 1000;
    vmHeader.codeOffset       = sizeof(vmHeader_t);
    vmHeader.codeLength       = 1024;
    vmHeader.dataOffset       = vmHeader.codeOffset + vmHeader.codeLength;
    vmHeader.dataLength       = 2048;
    vmHeader.litLength        = 0;
    vmHeader.bssLength        = 256;
    VM_Create(&vm, "test", (uint8_t*)&vmHeader,
              vmHeader.dataOffset + vmHeader.dataLength + vmHeader.litLength -
                  1,
              systemCalls);

    VM_Call(NULL, 0);

    VM_Free(NULL);

    vm.callLevel = 1;
    VM_Free(&vm);
}

int main(int argc, char** argv)
{
    const char* files[] = {"test.qvm", argv[argc > 1]};
    const char* file = files[argc > 1];

    testArguments();
    /* <malloc fail tests> */
    for (int i = 0; i < VM_ALLOC_TYPE_MAX - 1; i++)
    {
        g_mallocFail = i;
        testNominal(file);
    }
    g_mallocFail = -1;
    /* </malloc fail tests> */

    testInject(NULL, 0, 0);
    testNominal(NULL);
    testInject(file, 32, 0);
    testInject(file, 32, 63);
    testInject(file, 32, 65);
    testInject(file, 4, -1);
    /* finally: test the normal case */
    return testNominal(file);
}

void Com_Error(vmErrorCode_t level, const char* error)
{
    fprintf(stderr, "Err(%i): %s\n", level, error);
}

/* Callback from the VM for memory allocation */
void* Com_malloc(size_t size, vm_t* vm, vmMallocType_t type)
{
    (void)vm;
    (void)type;
    if (g_mallocFail != -1)
    {
        if (type == g_mallocFail)
        {
            return NULL;
        }
    }
    return malloc(size);
}

/* Callback from the VM for memory release */
void Com_free(void* p, vm_t* vm, vmMallocType_t type)
{
    (void)vm;
    (void)type;
    free(p);
}

uint8_t* loadImage(const char* filepath, int* size)
{
    FILE*    f;            /* bytecode input file */
    uint8_t* image = NULL; /* bytecode buffer */
    int      sz;           /* bytecode file size */

    if (!filepath || filepath[0] == '\0')
    {
        return NULL;
    }

    *size = 0;
    f     = fopen(filepath, "rb");
    if (!f)
    {
        fprintf(stderr, "Failed to open file %s.\n", filepath);
        return NULL;
    }
    /* calculate file size */
    fseek(f, 0L, SEEK_END);
    sz = ftell(f);
    /*
    if (sz < 1)
    {
        fclose(f);
        return NULL;
    }
    */
    rewind(f);

    image = (uint8_t*)malloc(sz);
    /*
    if (!image)
    {
        fclose(f);
        return NULL;
    }
    */

    fread(image, 1, sz, f);
    fclose(f);
    *size = sz;
    return image;
}

/* Callback from the VM: system function call */
intptr_t systemCalls(vm_t* vm, intptr_t* args)
{
    const int id = -1 - args[0];

    switch (id)
    {
    case -1: /* PRINTF */
        return printf("%s", (const char*)VMA(1, vm));

    case -2: /* ERROR */
        return fprintf(stderr, "%s", (const char*)VMA(1, vm));

    case -3: /* MEMSET */
        if (VM_MemoryRangeValid(args[1] /*addr*/, args[3] /*len*/, vm) == 0)
        {
            memset(VMA(1, vm), args[2], args[3]);
        }
        return args[1];

    case -4: /* MEMCPY */
        if (VM_MemoryRangeValid(args[1] /*addr*/, args[3] /*len*/, vm) == 0 &&
            VM_MemoryRangeValid(args[2] /*addr*/, args[3] /*len*/, vm) == 0)
        {
            memcpy(VMA(1, vm), VMA(2, vm), args[3]);
        }
        return args[1];

    case -6: /* FLOATFF */
        return VM_FloatToInt(VMF(1) * 2.0f);

    case -7: /* RECURSIVE */
        return VM_Call(vm, 1, args[1]);

    default:
        fprintf(stderr, "Bad system call: %ld\n", (long int)args[0]);
    }
    return 0;
}
