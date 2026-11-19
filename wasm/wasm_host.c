/* wasm_host.c -- the q3vm host for WebAssembly. It exports a small API:
 *
 *   alloc_bytes(n)      a buffer in the module memory for a .qvm image
 *   load(image, n)      loads the image, returns 0 on success
 *   call(command)       runs vmMain(command) and returns its result
 *
 * Build with wasm/Makefile (Emscripten). The image must have no system calls,
 * or the callback below must be extended. */

#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>

#include "vm.h"

static vm_t vm;

/* host callbacks required by the VM, see README.md */
void Com_Error(vmErrorCode_t level, const char* error)
{
    printf("VM error (%i): %s\n", level, error);
    exit(level);
}

void* Com_malloc(size_t size, vm_t* v, vmMallocType_t type)
{
    (void)v;
    (void)type;
    return malloc(size);
}

void Com_free(void* p, vm_t* v, vmMallocType_t type)
{
    (void)v;
    (void)type;
    free(p);
}

static intptr_t systemCalls(vm_t* v, intptr_t* args)
{
    (void)v;
    printf("unexpected system call %d\n", -1 - (int)args[0]);
    exit(1);
    return 0;
}

EMSCRIPTEN_KEEPALIVE
uint8_t* alloc_bytes(int n)
{
    return malloc(n);
}

EMSCRIPTEN_KEEPALIVE
int load(uint8_t* image, int n)
{
    return VM_Create(&vm, "wasm", image, n, systemCalls);
}

EMSCRIPTEN_KEEPALIVE
int call(int command)
{
    return (int)VM_Call(&vm, command);
}

/* a 64-bit result of vmMain, for bytecode64 images */
EMSCRIPTEN_KEEPALIVE
int64_t call64(int command)
{
    return VM_Call64(&vm, command);
}
