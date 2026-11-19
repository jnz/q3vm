/* lcc-run.c -- one executable that compiles a C program with the compiler running
 * in q3vm, links it with the VM runtime and runs it in q3vm.
 *
 * Usage: lcc-run file.c [arguments...]
 *
 * The executable holds everything it needs (payload.c, made by payload.py):
 * the compiler rcc.qvm, the assembler q3asm.qvm, the runtime library and the include
 * files of q3vm, and the preprocessor q3cpp. All of them are VM images. The files
 * of the steps are in the VM's virtual file system (vfs.c), not on the disk:
 *
 *   1. the payload, the source file and the headers next to it go into the VFS
 *   2. q3cpp.qvm, the preprocessor in the VM, preprocesses the source
 *   3. rcc.qvm, the compiler in the VM, writes LCC bytecode
 *   4. q3asm.qvm, the assembler in the VM, links the bytecode with the runtime library
 *   5. the program runs in the VM, with the arguments
 *
 * LCC_RUN_WIDTH (32 or 64) is set by the Makefile and must match rcc.qvm.
 */

#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "payload.h"
#include "vfs.h"
#include "vmhost.h"

#ifndef LCC_RUN_WIDTH
#define LCC_RUN_WIDTH 32
#endif

#if LCC_RUN_WIDTH == 64
#define TARGET_OPTION "-target=bytecode64"
#else
#define TARGET_OPTION "-target=bytecode"
#endif

/* the headers of q3vm leave out the 64-bit functions for a 32-bit target */
#if LCC_RUN_WIDTH == 64
#define CPP_DEFINES_WIDTH
#else
#define CPP_DEFINES_WIDTH "-DLCC_WIDE32",
#endif

static const char* cpp_defines[] = {
    "-D__STDC__=1", "-D__STRICT_ANSI__", "-D__signed__=signed", "-DQ3_VM", "-D__LCC__",
    CPP_DEFINES_WIDTH NULL};

static const struct payload_file* find_payload(const char* name)
{
    const struct payload_file* f;

    for (f = payload_files; f->name; f++)
    {
        if (strcmp(f->name, name) == 0)
        {
            return f;
        }
    }
    return NULL;
}

/* reads a whole host file, NULL when it cannot be read */
static uint8_t* read_file(const char* path, long* size)
{
    FILE*    f = fopen(path, "rb");
    uint8_t* data;

    if (!f)
    {
        return NULL;
    }
    if (fseek(f, 0, SEEK_END) != 0 || (*size = ftell(f)) < 0 || fseek(f, 0, SEEK_SET) != 0)
    {
        fclose(f);
        return NULL;
    }
    data = malloc(*size ? *size : 1);
    if (data && fread(data, 1, *size, f) != (size_t)*size)
    {
        free(data);
        data = NULL;
    }
    fclose(f);
    return data;
}

/* puts the payload into the VFS: the images, the runtime library and the headers */
static void put_payload(void)
{
    const struct payload_file* f;

    for (f = payload_files; f->name; f++)
    {
        vfs_put(f->name, f->data, f->size);
    }
}

/* puts the headers next to the source file in the VFS, so that quoted includes work */
static void put_headers(const char* srcdir)
{
    DIR*           d = opendir(srcdir);
    struct dirent* e;

    if (!d)
    {
        return;
    }
    while ((e = readdir(d)) != NULL)
    {
        size_t  len = strlen(e->d_name);
        char    path[4096];
        long    size;
        uint8_t* data;

        if (len < 3 || strcmp(e->d_name + len - 2, ".h") != 0)
        {
            continue;
        }
        snprintf(path, sizeof path, "%s/%s", srcdir, e->d_name);
        data = read_file(path, &size);
        if (data)
        {
            vfs_put(e->d_name, data, size);
            free(data);
        }
    }
    closedir(d);
}

static void usage(const char* name)
{
    fprintf(stderr, "usage: %s file.c [arguments...]\n", name);
    exit(2);
}

int main(int argc, char** argv)
{
    const char*                src;
    char                       srcdir[4096];
    char                       base[256];
    char                       base_c[260];
    char                       iname[260];
    char                       aname[260];
    const char*                name;
    const char*                slash;
    const char*                dot;
    const struct payload_file* cpp;
    const struct payload_file* rcc;
    const struct payload_file* qasm;
    char*                      cppargv[16];
    char*                      rargv[4];
    char*                      qargv[8];
    char**                     pargv;
    uint8_t*                   source;
    uint8_t*                   image;
    long                       size = 0;
    int                        status;
    int                        n;
    int                        i;

    if (argc < 2)
    {
        usage(argv[0]);
    }
    src = argv[1];
    slash = strrchr(src, '/');
    if (slash)
    {
        size_t len = slash - src;

        snprintf(srcdir, sizeof srcdir, "%.*s", (int)(len ? len : 1), src);
    }
    else
    {
        snprintf(srcdir, sizeof srcdir, ".");
    }
    name = slash ? slash + 1 : src;
    dot = strrchr(name, '.');
    snprintf(base, sizeof base, "%.*s", dot ? (int)(dot - name) : (int)strlen(name), name);
    snprintf(base_c, sizeof base_c, "%s.c", base);
    snprintf(iname, sizeof iname, "%s.i", base);
    snprintf(aname, sizeof aname, "%s.asm", base);

    /* 1. the files of the steps, in the VFS, relative names start from "/" */
    vfs_init("/", 0);
    put_payload();
    put_headers(srcdir);
    source = read_file(src, &size);
    if (!source)
    {
        fprintf(stderr, "lcc-run: cannot read %s\n", src);
        return 1;
    }
    vfs_put(base_c, source, size);
    free(source);

    /* 2. preprocess in the VM, the output file is the second file argument */
    cpp = find_payload("q3cpp.qvm");
    if (!cpp)
    {
        fprintf(stderr, "lcc-run: the preprocessor is missing from the executable\n");
        return 1;
    }
    n = 0;
    cppargv[n++] = "q3cpp";
    for (i = 0; cpp_defines[i]; i++)
    {
        cppargv[n++] = (char*)cpp_defines[i];
    }
    cppargv[n++] = "-Iinclude";
    cppargv[n++] = base_c;
    cppargv[n++] = iname;
    if (vmhost_run(cpp->data, (int)cpp->size, n, cppargv) != 0)
    {
        fprintf(stderr, "lcc-run: preprocessing failed\n");
        return 1;
    }

    /* 3. compile in the VM */
    rcc = find_payload("rcc.qvm");
    if (!rcc)
    {
        fprintf(stderr, "lcc-run: the compiler is missing from the executable\n");
        return 1;
    }
    rargv[0] = "rcc";
    rargv[1] = TARGET_OPTION;
    rargv[2] = iname;
    rargv[3] = aname;
    if (vmhost_run(rcc->data, (int)rcc->size, 4, rargv) != 0)
    {
        fprintf(stderr, "lcc-run: compilation failed\n");
        return 1;
    }

    /* 4. link with the runtime library, the assembler runs in the VM too */
    qasm = find_payload("q3asm.qvm");
    if (!qasm)
    {
        fprintf(stderr, "lcc-run: the assembler is missing from the executable\n");
        return 1;
    }
    qargv[0] = "q3asm";
    qargv[1] = "-o";
    qargv[2] = "prog";
    qargv[3] = "lib/vmmain";
    qargv[4] = "lib/vmlib";
    qargv[5] = "lib/bg_lib";
    qargv[6] = base;
    qargv[7] = "lib/vm_syscalls";
    if (vmhost_run(qasm->data, (int)qasm->size, 8, qargv) != 0)
    {
        fprintf(stderr, "lcc-run: linking failed\n");
        return 1;
    }

    /* 5. run the program in the VM, its argv[0] is the source name */
    size = vfs_size("prog.qvm");
    if (size < 0)
    {
        fprintf(stderr, "lcc-run: no program image\n");
        return 1;
    }
    image = malloc(size ? size : 1);
    if (!image || vfs_get("prog.qvm", image, size) != size)
    {
        fprintf(stderr, "lcc-run: cannot load the program image\n");
        return 1;
    }
    pargv = malloc(sizeof(char*) * (argc - 1));
    pargv[0] = (char*)base;
    for (i = 2; i < argc; i++)
    {
        pargv[i - 1] = argv[i];
    }
    fflush(stdout);
    status = vmhost_run(image, (int)size, argc - 1, pargv);
    free(image);
    free(pargv);
    return status < 0 ? 2 : status;
}
