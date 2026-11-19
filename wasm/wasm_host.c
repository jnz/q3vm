/* wasm_host.c -- the q3vm host for WebAssembly. It runs images with the same system
 * calls as the native host (lcc/vm/vmhost.c), on files in memory:
 *
 *   vm_init()                      the file system starts empty, in "/"
 *   alloc_bytes(n) / free_bytes(p) buffers in the module memory
 *   put_file(name, data, n)        a file for the VM, relative names start from "/"
 *   file_size(name)                the size of a file, -1 when there is none
 *   get_file(name, buf, n)         copies a file out, returns its size
 *   run(image, n, argc, args)      runs main(argc, argv) of the image; args holds argc
 *                                  NUL-terminated strings one after the other.
 *                                  Returns the result of main, or -1 when the image
 *                                  does not load.
 *
 * Build with wasm/Makefile (Emscripten). */

#include <emscripten.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "vfs.h"
#include "vmhost.h"

EMSCRIPTEN_KEEPALIVE
void vm_init(void)
{
    vfs_init("/", 0);
}

EMSCRIPTEN_KEEPALIVE
uint8_t* alloc_bytes(int n)
{
    return malloc(n);
}

EMSCRIPTEN_KEEPALIVE
void free_bytes(uint8_t* p)
{
    free(p);
}

EMSCRIPTEN_KEEPALIVE
void put_file(const char* name, const uint8_t* data, int n)
{
    vfs_put(name, data, n);
}

EMSCRIPTEN_KEEPALIVE
long file_size(const char* name)
{
    return vfs_size(name);
}

EMSCRIPTEN_KEEPALIVE
long get_file(const char* name, uint8_t* buf, int n)
{
    return vfs_get(name, buf, n);
}

EMSCRIPTEN_KEEPALIVE
int run(const uint8_t* image, int size, int argc, const char* args)
{
    char**      argv = malloc(sizeof(char*) * (argc > 0 ? argc : 1));
    const char* p = args;
    int         i;
    int         status;

    if (!argv)
    {
        return -1;
    }
    for (i = 0; i < argc; i++)
    {
        argv[i] = (char*)p;
        p += strlen(p) + 1;
    }
    status = vmhost_run(image, size, argc, argv);
    free(argv);
    return status;
}
