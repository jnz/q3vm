/* driver.c -- the steps of lcc-run, written for the VM. The driver is itself a VM
 * image (driver.qvm, built by the Makefile), and lcc-run runs it on the files of the
 * virtual file system. It does what lcc-run used to do natively:
 *
 *   1. q3cpp.qvm preprocesses the source
 *   2. rcc.qvm compiles the output to LCC bytecode
 *   3. q3asm.qvm links the bytecode with the runtime library
 *   4. the program runs, with the arguments
 *
 * Usage: driver [-S | -c] file.c [arguments...]
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

int main(int argc, char** argv)
{
    char* cppargv[16];
    char* rargv[4];
    char* qargv[8];
    char* pargv[64];
    char  base[256];
    char  iname[260];
    char  aname[260];
    char* name;
    char* dot;
    int   n;
    int   i;
    int   status;
    int   first = 1; /* argv[first] is the source file */
    int   compileOnly = 0;
    int   linkOnly = 0;

    /* -S stops after the compiler, -c after the linker: the program is not run */
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
        else
        {
            break;
        }
        first++;
    }
    if (argc <= first || argc - first > 64)
    {
        fprintf(stderr, "usage: driver [-S | -c] file.c [arguments...]\n");
        return 2;
    }
    name = strrchr(argv[first], '/');
    name = name ? name + 1 : argv[first];
    dot = strrchr(name, '.');
    n = dot ? (int)(dot - name) : (int)strlen(name);
    if (n >= (int)sizeof base)
    {
        n = sizeof base - 1;
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

    if (compileOnly)
    {
        return 0;
    }

    /* 3. link with the runtime library, its argv[0] is the name of the source */
    qargv[0] = "q3asm";
    qargv[1] = "-o";
    qargv[2] = "prog";
    qargv[3] = "lib/vmmain";
    qargv[4] = "lib/vmlib";
    qargv[5] = "lib/bg_lib";
    qargv[6] = base;
    qargv[7] = "lib/vm_syscalls";
    if (vm_run("q3asm.qvm", 8, qargv) != 0)
    {
        fprintf(stderr, "lcc-run: linking failed\n");
        return 1;
    }
    if (linkOnly)
    {
        return 0;
    }

    /* 4. run the program, with the source name as its argv[0] */
    pargv[0] = base;
    for (i = first + 1; i < argc; i++)
    {
        pargv[i - first] = argv[i];
    }
    status = vm_run("prog.qvm", argc - first, pargv);
    return status < 0 ? 2 : status;
}
