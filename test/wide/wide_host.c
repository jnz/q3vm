/*
 * Host side of the 64-bit test: runs every vmMain command of wide_test.qvm in
 * the VM and natively (wide_test.c is included here) and compares the results.
 *
 * Build and run: see the Makefile in this folder.
 */
#include <stdio.h>
#include <stdlib.h>

#include "vm.h"
#include "wide_test.c"

/* host callbacks required by the VM, see README.md */
void Com_Error(vmErrorCode_t level, const char* error)
{
    fprintf(stderr, "Err (%i): %s\n", level, error);
    exit(level);
}

void* Com_malloc(size_t size, vm_t* vm, vmMallocType_t type)
{
    (void)vm;
    (void)type;
    return malloc(size);
}

void Com_free(void* p, vm_t* vm, vmMallocType_t type)
{
    (void)vm;
    (void)type;
    free(p);
}

/* native functions called by the bytecode, the native run calls them too */
i64 wide_add(i64 a, i64 b)
{
    return a + b;
}

double wide_scale(i64 a, double d)
{
    return (double)a * d;
}

/* args[0] is -1 - syscall id, the 64-bit arguments follow as word pairs */
intptr_t systemCalls(vm_t* vm, intptr_t* args)
{
    union {
        double    d;
        intptr_t  i;
    } result;

    (void)vm;
    switch (-1 - (int)args[0])
    {
    case -5:
        return (intptr_t)wide_add(VMI64(1), VMI64(3));
    case -6:
        result.d = wide_scale(VMI64(1), VM_Int64ToDouble(VMI64(3)));
        return result.i;
    default:
        fprintf(stderr, "unexpected system call %d\n", -1 - (int)args[0]);
        exit(1);
    }
}

static uint8_t* readFile(const char* path, int* size)
{
    FILE*    f = fopen(path, "rb");
    uint8_t* data;

    if (!f)
    {
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    *size = (int)ftell(f);
    fseek(f, 0, SEEK_SET);
    data = malloc(*size);
    if (data && fread(data, 1, *size, f) != (size_t)*size)
    {
        free(data);
        data = NULL;
    }
    fclose(f);
    return data;
}

int main(int argc, char** argv)
{
    static const int commands = 7;
    vm_t             vm;
    int              size = 0;
    uint8_t*         image;
    int              cmd;
    int              failures = 0;

    if (argc < 2)
    {
        fprintf(stderr, "usage: %s wide_test.qvm\n", argv[0]);
        return 2;
    }
    image = readFile(argv[1], &size);
    if (!image)
    {
        fprintf(stderr, "cannot read %s\n", argv[1]);
        return 2;
    }
    if (VM_Create(&vm, argv[1], image, size, systemCalls) != 0)
    {
        fprintf(stderr, "VM_Create failed\n");
        return 2;
    }

    for (cmd = 0; cmd < commands; cmd++)
    {
        i64       native = vmMain(cmd);
        intptr_t  bytecode = VM_Call(&vm, cmd);
        int       same = (native == (i64)bytecode);

        printf("%s command %d: native %lld, vm %lld\n", same ? "ok  " : "FAIL",
               cmd, native, (i64)bytecode);
        if (!same)
        {
            failures++;
        }
    }

    VM_Free(&vm);
    free(image);
    printf("%d of %d cases differ\n", failures, commands);
    return failures ? 1 : 0;
}
