/* driver.c -- the steps of lcc-run, written for the VM. The driver is itself a VM
 * image (driver.qvm, built by the Makefile), and lcc-run runs it on the files of the
 * virtual file system. It does what lcc-run used to do natively:
 *
 *   1. q3cpp.qvm preprocesses each source
 *   2. rcc.qvm compiles each output to LCC bytecode
 *   3. q3asm.qvm links the bytecode with the runtime library
 *   4. the program runs, with the arguments
 *
 * Usage: driver [-S | -c] [-s file.c]... file.c [arguments...]
 * -s adds a source file to the program, compiled with the others and linked with them.
 * -S stops after the compiler, -c after the linker: the linked image is prog.qvm in
 * the file system, and the program is not run.
 *
 * Each tool is run with vm_run, a system call that runs an image of the file system.
 * LCC_RUN_WIDTH (32 or 64) is set by the Makefile and must match rcc.qvm.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vmsys.h"

#ifndef LCC_RUN_WIDTH
#define LCC_RUN_WIDTH 32
#endif

/* the headers of q3vm leave out the 64-bit functions for a 32-bit target */
#if LCC_RUN_WIDTH == 64
#define TARGET_OPTION "-target=bytecode64"
#define CPP_DEFINES_WIDTH
#else
#define TARGET_OPTION "-target=bytecode"
#define CPP_DEFINES_WIDTH "-DLCC_WIDE32",
#endif

static char* cpp_defines[] = {
    "-D__STDC__=1", "-D__STRICT_ANSI__", "-D__signed__=signed", "-DQ3_VM", "-D__LCC__",
    CPP_DEFINES_WIDTH NULL};

#define MAX_SOURCES 16

/* preprocesses and compiles the source file path (its name in the file system is its
 * base name): base.asm is its assembly. base receives the name without the extension.
 * Returns 0 when it is done. */
static int compileSource(const char* path, char* base)
{
    char* cppargv[16];
    char* rargv[4];
    char  iname[260];
    char  aname[260];
    char* name;
    char* dot;
    int   n;
    int   i;

    name = strrchr(path, '/');
    name = name ? name + 1 : (char*)path;
    dot  = strrchr(name, '.');
    n    = dot ? (int)(dot - name) : (int)strlen(name);
    if (n >= 256)
    {
        n = 255;
    }
    memcpy(base, name, n);
    base[n] = '\0';
    sprintf(iname, "%s.i", base);
    sprintf(aname, "%s.asm", base);

    /* 1. preprocess, the output file is the second file argument. The files are in
     * the directory of the VM, so the source is named without its directory: the
     * -I directories are relative to the directory of the source. */
    n = 0;
    cppargv[n++] = "q3cpp";
    for (i = 0; cpp_defines[i]; i++)
    {
        cppargv[n++] = cpp_defines[i];
    }
    cppargv[n++] = "-Iinclude";
    cppargv[n++] = name;
    cppargv[n++] = iname;
    if (vm_run("q3cpp.qvm", n, cppargv) != 0)
    {
        fprintf(stderr, "lcc-run: preprocessing failed\n");
        return 1;
    }

    /* 2. compile */
    rargv[0] = "rcc";
    rargv[1] = TARGET_OPTION;
    rargv[2] = iname;
    rargv[3] = aname;
    if (vm_run("rcc.qvm", 4, rargv) != 0)
    {
        fprintf(stderr, "lcc-run: compilation failed\n");
        return 1;
    }
    return 0;
}

int main(int argc, char** argv)
{
    char*  qargv[8 + MAX_SOURCES];
    char*  pargv[64];
    char   bases[MAX_SOURCES][256];
    char*  sources[MAX_SOURCES];
    int    nsources = 0;
    int    n;
    int    i;
    int    status;
    int    first = 1; /* argv[first] is the source file */
    int    compileOnly = 0;
    int    linkOnly = 0;

    /* -S stops after the compiler, -c after the linker: the program is not run.
     * -s adds a source file, and the first source is the one after the options. */
    while (argc > first + 1 && argv[first][0] == '-')
    {
        if (strcmp(argv[first], "-S") == 0)
        {
            compileOnly = 1;
        }
        else if (strcmp(argv[first], "-c") == 0)
        {
            linkOnly = 1;
        }
        else if (strcmp(argv[first], "-s") == 0 && argc > first + 2 && nsources < MAX_SOURCES - 1)
        {
            sources[++nsources] = argv[first + 1];
            first++;
        }
        else
        {
            break;
        }
        first++;
    }
    if (argc <= first || argc - first > 64)
    {
        fprintf(stderr, "usage: driver [-S | -c] [-s file.c]... file.c [arguments...]\n");
        return 2;
    }
    sources[0] = argv[first];
    nsources++;

    /* 1-2. each source is preprocessed and compiled to base.asm */
    for (i = 0; i < nsources; i++)
    {
        if (compileSource(sources[i], bases[i]) != 0)
        {
            return 1;
        }
    }
    if (compileOnly)
    {
        return 0;
    }

    /* 3. link with the runtime library, its argv[0] is the name of the first source */
    n = 0;
    qargv[n++] = "q3asm";
    qargv[n++] = "-o";
    qargv[n++] = "prog";
    qargv[n++] = "lib/vmmain";
    qargv[n++] = "lib/vmlib";
    qargv[n++] = "lib/bg_lib";
    for (i = 0; i < nsources; i++)
    {
        qargv[n++] = bases[i];
    }
    qargv[n++] = "lib/vm_syscalls";
    if (vm_run("q3asm.qvm", n, qargv) != 0)
    {
        fprintf(stderr, "lcc-run: linking failed\n");
        return 1;
    }
    if (linkOnly)
    {
        return 0;
    }

    /* 4. run the program, with the first source name as its argv[0] */
    pargv[0] = bases[0];
    for (i = first + 1; i < argc; i++)
    {
        pargv[i - first] = argv[i];
    }
    status = vm_run("prog.qvm", argc - first, pargv);
    return status < 0 ? 2 : status;
}
