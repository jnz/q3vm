/* vmhost.c -- the host side of the VM: its system calls, files and the runner.
 *
 * The VM has no files of its own. Its file calls go to the VFS (vfs.c), its
 * environment and time calls are served here, and the arguments of a run are
 * passed to the image's main(). The same code runs natively and in WebAssembly.
 */

#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "vfs.h"
#include "vm.h"
#include "vmhost.h"

#define VM_MAX_FILES 64

/* handles 0 to 2 are the host's standard streams, the others are files of the VFS */
static vfs_file_t* host_files[VM_MAX_FILES];
static int         host_argc;
static char**      host_argv;
/* exit() in the VM returns to the innermost vmhost_run, see there */
static jmp_buf* host_exit;
static int      host_exit_status;
/* the heap: -heap adds this many bytes to the bss of each image, and the heap of the
 * image runs from heap_start to heap_end (VM addresses, 0 when there is no extra heap) */
static size_t   host_heap_extra;
static intptr_t host_heap_start;
static intptr_t host_heap_end;

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

static int freeHostFile(void)
{
    int i;

    for (i = 3; i < VM_MAX_FILES; i++)
    {
        if (!host_files[i])
        {
            return i;
        }
    }
    return -1;
}

static FILE* stdStream(int handle)
{
    switch (handle)
    {
    case 0:
        return stdin;
    case 1:
        return stdout;
    case 2:
        return stderr;
    default:
        return NULL;
    }
}

static int isFile(int handle)
{
    return handle >= 3 && handle < VM_MAX_FILES && host_files[handle];
}

static size_t hostRead(int handle, void* buf, size_t n)
{
    if (isFile(handle))
    {
        return vfs_read(host_files[handle], buf, n);
    }
    return fread(buf, 1, n, stdStream(handle));
}

static size_t hostWrite(int handle, const void* buf, size_t n)
{
    if (isFile(handle))
    {
        return vfs_write(host_files[handle], buf, n);
    }
    return fwrite(buf, 1, n, stdStream(handle));
}

/* a bad VM pointer is an error, not a silent wrong result. VM_MemoryRangeValid
 * returns 0 when the range is valid. */
static void* vmPtr(vm_t* vm, intptr_t addr, intptr_t len)
{
    if (VM_MemoryRangeValid(addr, len, vm) != 0)
    {
        Com_Error(VM_INVALID_POINTER, "syscall argument outside the VM memory");
    }
    return VM_ArgPtr(addr, vm);
}

/* args[0] is -1 - syscall id, the parameters follow in args[1..] */
intptr_t vmhost_syscalls(vm_t* vm, intptr_t* args)
{
    switch (-1 - (int)args[0])
    {
    case -1: /* vm_open(path, mode) */
    {
        const char*  path = vmPtr(vm, args[1], 1);
        const char*  mode = vmPtr(vm, args[2], 1);
        int          slot = freeHostFile();
        vfs_file_t*  f;

        if (slot < 0)
        {
            return -1;
        }
        f = vfs_open(path, mode);
        if (!f)
        {
            return -1;
        }
        host_files[slot] = f;
        return slot;
    }
    case -2: /* vm_close(handle) */
    {
        int handle = (int)args[1];

        if (isFile(handle))
        {
            vfs_close(host_files[handle]);
            host_files[handle] = NULL;
        }
        return 0;
    }
    case -3: /* vm_read(handle, buf, n) */
    {
        int    n = (int)args[3];
        size_t got;

        if (!isFile((int)args[1]) && !stdStream((int)args[1]))
        {
            return -1;
        }
        got = hostRead((int)args[1], vmPtr(vm, args[2], n), n);
        VM_Poison(vm, args[2], (int)got, 1);
        return (intptr_t)got;
    }
    case -4: /* vm_write(handle, buf, n) */
    {
        int n = (int)args[3];

        if (!isFile((int)args[1]) && !stdStream((int)args[1]))
        {
            return -1;
        }
        return (intptr_t)hostWrite((int)args[1], vmPtr(vm, args[2], n), n);
    }
    case -5: /* vm_seek(handle, off, whence) */
    {
        int handle = (int)args[1];

        if (isFile(handle))
        {
            return vfs_seek(host_files[handle], (long)args[2], (int)args[3]) == 0 ? 0 : -1;
        }
        if (!stdStream(handle))
        {
            return -1;
        }
        return fseek(stdStream(handle), (long)args[2], (int)args[3]) == 0 ? 0 : -1;
    }
    case -6: /* vm_tell(handle) */
    {
        int handle = (int)args[1];

        if (isFile(handle))
        {
            return (intptr_t)vfs_tell(host_files[handle]);
        }
        if (!stdStream(handle))
        {
            return -1;
        }
        return (intptr_t)ftell(stdStream(handle));
    }
    case -7: /* vm_exit(status) */
        host_exit_status = (int)args[1];
        longjmp(*host_exit, 1);
    case -8: /* vm_getenv(name, buf, n) */
    {
        const char* value = getenv(vmPtr(vm, args[1], 1));
        int         n = (int)args[3];
        char*       buf = vmPtr(vm, args[2], n);

        if (!value)
        {
            return -1;
        }
        strncpy(buf, value, n - 1);
        buf[n - 1] = 0;
        VM_Poison(vm, args[2], (int)strlen(buf) + 1, 1);
        return (intptr_t)strlen(value);
    }
    case -9: /* vm_time() */
        return (intptr_t)time(NULL);
    case -10: /* vm_ctime(t, buf, n) */
    {
        time_t      t = (time_t)args[1];
        const char* text = ctime(&t);
        int         n = (int)args[3];
        char*       buf = vmPtr(vm, args[2], n);

        strncpy(buf, text, n - 1);
        buf[n - 1] = 0;
        VM_Poison(vm, args[2], (int)strlen(buf) + 1, 1);
        return 0;
    }
    case -17: /* vm_heap_start(): the first address of the extra heap, 0 for none */
        return host_heap_start;
    case -18: /* vm_heap_end(): the end of the extra heap, below the stack */
        return host_heap_end;
    case -11: /* vm_argc() */
        return host_argc;
    case -12: /* vm_argv(i, buf, n) */
    {
        int   i = (int)args[1];
        int   n = (int)args[3];
        char* buf = vmPtr(vm, args[2], n);

        if (i < 0 || i >= host_argc)
        {
            return -1;
        }
        strncpy(buf, host_argv[i], n - 1);
        buf[n - 1] = 0;
        VM_Poison(vm, args[2], (int)strlen(buf) + 1, 1);
        return (intptr_t)strlen(host_argv[i]);
    }
    case -13: /* vm_remove(path) */
        return vfs_remove(vmPtr(vm, args[1], 1)) == 0 ? 0 : -1;
    case -14: /* vm_poison(p, n): memory that is not written yet, see VM_CHECKED */
        VM_Poison(vm, args[1], (int)args[2], 0);
        return 0;
    case -15: /* vm_strtod(s, out): the C library conversion, so the result is exact. The
                 result is always the 8 bytes of a double, whatever the target of the image
                 (a 32-bit image has floats): the VM builds its value from the bits. */
    {
        double* out = vmPtr(vm, args[2], sizeof(double));

        *out = strtod(vmPtr(vm, args[1], 1), NULL);
        VM_Poison(vm, args[2], sizeof(double), 1);
        return 0;
    }
    case -16: /* vm_run(path, argc, argv): runs an image of the VFS as a program, with
                 argv an array of argc strings of the VM. Returns its result. */
    {
        const char* path = vmPtr(vm, args[1], 1);
        int         argc = (int)args[2];
        intptr_t    argvAddr = args[3];
        long        size = vfs_size(path);
        uint8_t*    image;
        char**      argv;
        int         i;
        int         status;

        if (size < 0 || argc < 1)
        {
            return -1;
        }
        image = malloc(size ? size : 1);
        argv = malloc(sizeof(char*) * argc);
        if (!image || !argv)
        {
            free(image);
            free(argv);
            return -1;
        }
        vfs_get(path, image, size);
        for (i = 0; i < argc; i++)
        {
            int32_t addr;

            memcpy(&addr, vmPtr(vm, argvAddr + 4 * i, 4), 4);
            argv[i] = vmPtr(vm, addr, 1);
        }
        status = vmhost_run(image, (int)size, argc, argv);
        free(image);
        free(argv);
        return status;
    }
    default:
        fprintf(stderr, "unexpected system call %d\n", -1 - (int)args[0]);
        exit(1);
    }
}

/* sets the arguments that vm_argc and vm_argv return */
void vmhost_init(int argc, char** argv)
{
    host_argc = argc;
    host_argv = argv;
}

static int32_t le32(const uint8_t* p)
{
    return (int32_t)((uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24);
}

static void put_le32(uint8_t* p, int32_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

/* a copy of an image whose bss is extra bytes longer: the bytes after the image's data
 * are the heap. *heap is set to their first address. NULL when the image cannot be copied. */
static uint8_t* image_with_heap(const uint8_t* image, int size, size_t extra, intptr_t* heap)
{
    uint8_t* copy;
    int32_t  data;
    int32_t  lit;
    int32_t  bss;

    if (size < (int)sizeof(vmHeader_t))
    {
        return NULL;
    }
    data = le32(image + offsetof(vmHeader_t, dataLength));
    lit  = le32(image + offsetof(vmHeader_t, litLength));
    bss  = le32(image + offsetof(vmHeader_t, bssLength));
    copy = malloc(size);
    if (!copy)
    {
        return NULL;
    }
    memcpy(copy, image, size);
    put_le32(copy + offsetof(vmHeader_t, bssLength), (int32_t)(bss + extra));
    *heap = ((intptr_t)data + lit + bss + 7) & ~(intptr_t)7;
    return copy;
}

int vmhost_option(const char* arg)
{
    char*         end;
    unsigned long n;

    if (strncmp(arg, "-heap=", 6) != 0)
    {
        return 0;
    }
    n = strtoul(arg + 6, &end, 10);
    if (end == arg + 6)
    {
        return -1;
    }
    if (*end == 'K' || *end == 'k')
    {
        n *= 1024;
        end++;
    }
    else if (*end == 'M' || *end == 'm')
    {
        n *= 1024 * 1024;
        end++;
    }
    if (*end != 0 || n > 0x7fffffffUL - VM_PROGRAM_STACK_SIZE)
    {
        return -1;
    }
    host_heap_extra = (size_t)n;
    return 1;
}

/* runs a VM image: main(argc, argv) of the image, with the given arguments.
 * Returns the result of main, or -1 when the image does not load. */
int vmhost_run(const uint8_t* image, int size, int argc, char** argv)
{
    vm_t      vm;
    intptr_t  status;
    jmp_buf   exit_here;
    jmp_buf*  outer_exit = host_exit;
    int       outer_argc = host_argc;
    char**    outer_argv = host_argv;
    intptr_t  outer_heap_start = host_heap_start;
    intptr_t  outer_heap_end = host_heap_end;
    uint8_t*  patched = NULL;
    intptr_t  heap = 0;

    host_heap_start = 0;
    host_heap_end = 0;
    if (host_heap_extra > 0)
    {
        patched = image_with_heap(image, size, host_heap_extra, &heap);
        if (!patched)
        {
            return -1;
        }
        image = patched;
    }
    if (VM_Create(&vm, "vm", image, size, vmhost_syscalls) != 0)
    {
        free(patched);
        host_heap_start = outer_heap_start;
        host_heap_end = outer_heap_end;
        return -1;
    }
    /* the heap is the memory after the data, up to the stack */
    if (patched && (intptr_t)vm.dataMask + 1 - VM_PROGRAM_STACK_SIZE > heap)
    {
        host_heap_start = heap;
        host_heap_end = (intptr_t)vm.dataMask + 1 - VM_PROGRAM_STACK_SIZE;
    }
    /* a run can start another run (vm_run), so the state of the outer one is kept */
    vmhost_init(argc, argv);
    host_exit = &exit_here;
    /* the image's exit() jumps back here, so that the host finishes its own work */
    if (setjmp(exit_here))
    {
        /* the jump skipped the end of VM_Call, so the VM still counts it as running */
        vm.callLevel = 0;
        vm.currentlyInterpreting = 0;
        status = host_exit_status;
    }
    else
    {
        status = VM_Call(&vm, 0);
    }
    host_exit = outer_exit;
    host_argc = outer_argc;
    host_argv = outer_argv;
    host_heap_start = outer_heap_start;
    host_heap_end = outer_heap_end;
    /* files the image wrote and did not close must reach the disk now */
    vfs_flush();
    fflush(NULL);
    VM_Free(&vm);
    free(patched);
    return (int)status;
}
