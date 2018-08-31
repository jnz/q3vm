Q3VM Readme
===========

A lightweight (single file: `vm.c`) embeddable interpreter/Virtual Machine (VM) for compiled bytecode files (`.qvm`) based on good old C-language input (`.c`). A complete C compiler to generate `.qvm` files is included (LCC). The interpreter is based on the Quake III Arena virtual machine (hence the name q3vm) but the interpreter is not tied to Quake III Arena and can be used for any kind of project. For example code that needs to run in a sandbox.

      ___   _______     ____  __
     / _ \ |___ /\ \   / /  \/  |
    | | | |  |_ \ \ \ / /| |\/| |
    | |_| |____) | \ V / | |  | |
     \__\_______/   \_/  |_|  |_|

[![Build Status](https://travis-ci.org/jnz/q3vm.svg?branch=master)](https://travis-ci.org/jnz/q3vm)
[![codecov](https://codecov.io/gh/jnz/q3vm/branch/master/graph/badge.svg)](https://codecov.io/gh/jnz/q3vm)
[![Coverity](https://scan.coverity.com/projects/16570/badge.svg)](https://scan.coverity.com/projects/jnz-q3vm)

Jan Zwiener, 2018. Mail: jan@zwiener.org

Features
--------

 * Small and lightweight (one .c file to include without dependencies)
 * Battle-tested (20 years of use in Quake III Arena)
 * VM forked from the well maintained ioquake3 code base
 * Tool tested (static code analysis, test coverage, Valgrind)
 * No need to learn a new scripting language (e.g. Lua)
 * Strong typing in the scripting language (C)
 * Static memory allocation in C, no unpredictable garbage collector
 * Plan B: you can always go back to native code, as .c files are the input
 * Great tool landscape for C. Use the tools that are available for C
 * Computed gotos are used to speed up the interpreter if you compile with GCC (see benchmark section)

Use Cases
---------

 * Sandbox for code you don't fully trust (e.g. download the bytecode from a web server)
 * Mods for small hobby game engines
 * There are many virtual machines, but not many are so small, with strong typing and no garbage collector
 * Learn about virtual machines in general, but directly have a C compiler available for the virtual machine
 * Sandbox for embedded applications, e.g. plug-ins for IoT applications on microcontrollers (bounded CPU time, bounded memory area, restrict access to peripheral devices)
 * There is also a historical value: learn about the Quake III engine

Quick Intro
-----------

Two things are required:

 * The interpreter
 * A bytecode binary .qvm file

Run:

    > q3vm.exe bytecode.qvm

The q3vm.exe standalone interpreter is not required. You can easily add the
interpreter as a single .c file to your project (`vm.c` and the header `vm.h`).
Call `VM_Create` and `VM_Call` to run the bytecode in your application:

    #include "vm.h"

    vm_t vm;
    int result;
    
    VM_Create(&vm, "my test", pointerToByteCodeBuffer, sysCall);
    result = VM_Call(&vm, 12345);
    VM_Free(&vm);
    
The `pointerToByteCodeBuffer` is some memory location where the bytecode is
located. You can e.g. load it from a file and store it in a byte array. See
`main.c` for an example implementation.

Data can be exchanged with the bytecode by the return value (result) and
arguments to `VM_Call`. Here just a 1234 is passed to the bytecode. It is up to
the `vmMain` function in the bytecode what to do with that 0.  You can pass
more (up to 12) optional arguments to the bytecode:
e.g. `VM_Call(&vm, 0, 1, 2, 3, 4)`.

The `sysCall` is a callback function that you define so that the interpreter
can call native functions from your code. E.g. a logging function or some time
critical function that you don't want to implement in the bytecode. Again,
check `main.c` for an example. Also check the section *How to add a custom native function* for more information.

A few callback functions are required, read the section *Callback functions required in host application* for more information.

And normally you should also check if `VM_Create` returns 0 (i.e. everything is
OK).

Folder structure
----------------

    ├─ bin/
    │  ├─ linux/        Target folder for Linux lcc and q3asm builds
    │  └─ win32/        Precompiled lcc.exe and q3asm.exe for Windows
    ├─ build/           Target folder for q3vm build
    ├─ doxygen/         Doxygen config and output
    ├─ example/         Example bytecode.qvm
    ├─ lcc/             The LCC compiler (compile .c files to .asm files)
    ├─ msvc/            Microsoft Visual Studio 2015 project file for q3vm
    ├─ q3asm/           Link the LCC .asm to a .qvm bytecode file
    ├─ src/             q3vm standalone application source code
    │  └─ vm/           The core VM source, copy that folder into your project
    └─ test/            test bytecode qvm

API Documentation
-----------------

Call `make doxygen` to autogenerate the API documentation in the `doxygen/html`
directory. Doxygen is required as well as the dot command (part of graphviz).
Install it with `sudo apt-get install doxygen graphviz` on Debian or Ubuntu.

    > make doxygen

But you can also read `vm.h` directly for the API documentation.


Build VM/interpreter
--------------------

On **Linux**:

    > make

On **Windows**:

Use the **Visual Studio 2015** project `q3vm.sln` in the `msvc` subfolder.

Or install MinGW64 and add the MinGW64 bin\ directory to your path.
So that you have gcc.exe and mingw32-make.exe available at the command prompt.

 * http://mingw-w64.org/doku.php/download/mingw-builds

Compile with:

    > mingw32-make

Build example bytecode firmware
-------------------------------

**Windows**:

The LCC compiler (lcc.exe) is included in the ./bin/win32 directory.
You need make (mingw32-make) from the MinGW64 installation in
your path. The Makefile calls LCC and q3asm to generate `bytecode.qvm`:

    cd example
    mingw32-make

If you don't want to use make, you can do the steps from the make file
manually at the command line. Compile every `.c` source code with `LCC`:

    > lcc -S -Wf-target=bytecode -Wf-g YOUR_C_CODE.c

This will create .asm output files. Then link the .asm files with `q3asm` (based on a bytecode.q3asm
linker script):

    > q3asm -f bytecode

The output of q3asm is a `.qvm` file that you can run with q3vm.


**Linux**:

Build LCC:

    > make lcc

Build q3asm

    > make q3asm

Build the example bytecode:

    > make example/bytecode.qvm

Callback functions required in host application
-----------------------------------------------

**malloc and free**:

The following functions are required in the host application for
memory allocation:

    void* Com_malloc(size_t size, vm_t* vm, vmMallocType_t type);
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

The host can simply call `malloc` and `free` or use a custom memory allocation
function or use static memory (e.g. in an embedded application). Each VM only
calls `Com_malloc` once per malloc type. This can be used as a help for the static memory
allocation in an embedded environment without malloc() and free().

**Error handling**:

The following function needs to be implemented in the host application:

    void Com_Error(vmErrorCode_t level, const char* error)
    {
        fprintf(stderr, "Err (%i): %s\n", level, error);
        exit(level);
    }

The error id is given by the `vmErrorCode_t` parameter. The `error` string describes
what went wrong.  It is up to the host application how to deal with the error.
In this simple example we just print the error string and exit the application.
The error code is stored in the `vm_t::lastError` variable.

How to add a custom native function
-----------------------------------

Let's say we want to add a native function to convert a string to an integer:
`stringToInt`.  We want to add the function to our virtual machine (step 1) and
call it from our example code (step 2).  (Note: there is already the atoi function in
the bytecode, but this is just an example on how to call atoi as a native
function and deal with address translation) 


**Step 1)** Add the native function to the host application

Open `src/main.c` and modify the `systemCalls` function. Add `case -5:` for the
new native function. We just use the next free id (here -5) as an identifier.
The identifier will be important in step 2.  The first argument
for `stringToInt` is the address of a string. The address is in the virtual
machine address space, so we can't directly use that argument (`args[1]`) for
the native call to `atoi`. There is a helper macro that will translate the
address for use: `VMA`. We need to give `VMA` the pointer argument from the
bytecode and the virtual machine context (`vm`) to translate it.

    /* Call native functions from the bytecode: */
    intptr_t systemCalls(vm_t* vm, intptr_t* args)
    {
        int id = -1 - args[0];
    
        switch (id)
        {
        case -1: /* PRINTF */
            printf("%s", (const char*)VMA(1, vm));
            return 0;
        case -2: /* ERROR */
            fprintf(stderr, "%s", (const char*)VMA(1, vm));
            return 0;
    
        case -3: /* MEMSET */
            memset(VMA(1, vm), args[2], args[3]);
            return 0;
    
        case -4: /* MEMCPY */
            memcpy(VMA(1, vm), VMA(2, vm), args[3]);
            return 0;

        case -5: /* stringToInt */                             // < NEW
            return atoi(VMA(1, vm));                           // < NEW
    
        default:
            fprintf(stderr, "Bad system call: %ld", (long int)args[0]);
        }
        return 0;
    }

**Step 2)** Tell the bytecode about this function

Now we need to tell our example project about this new function `strintToInt`.
Open `example/g_syscalls.asm` and add the last line. The identifier -5 is
important for the mapping.

    code
    
    equ	trap_Printf				-1
    equ	trap_Error				-2
    equ	memset					-3
    equ	memcpy					-4
    equ	stringToInt				-5

**Step 3)** Perform an example call to `strintToInt`

Edit `example/main.c` and add the function declaration:

    int stringToInt(const char* a);

And call it somewhere from the `vmMain` function:

    char* myStr = "1234";
    printf("\"%s\" -> %i\n", myStr, stringToInt(myStr));

Compile everything:

    > make && make example/bytecode.qvm

And run it:

    > ./q3vm example/bytecode.qvm


Static code analysis
--------------------

Call `make analysis` and `make valgrind` to check the VM with:

 * clang static code analysis (scan-build)
 * cppcheck
 * Valgrind

clang-format
------------

Run the following command to reformat a file according to the coding style:

    > clang-format -i -style=file input.c


Debugging
---------

Build `vm.c` with `#define DEBUG_VM` in `vm.h` to enable more checks and debug
functions. Call `VM_Debug(1)` or `VM_Debug(2)` to enable debug printfs.  This
requires the symbol file of the `.qvm`: the `.map` file in the same directory
as the `.qvm`. The `.map` file is automatically generated for each `.qvm`.

Call at the end of a session `VM_VmProfile_f(vm)` to see a VM usage summary.

Benchmarks
----------

Time to run `test/test.qvm`:


| Interpreter          | Time     |
|----------------------|----------|
| Default Interpreter  |  3.063 s |
| w. computed gotos    |  1.771 s |
| Native executable    |  0.307 s |


Environment:

 * Ubuntu 17.10
 * GCC: 7.2.0-8ubuntu3.2
 * CPU: Intel(R) Core(TM) i5-3320M CPU @ 2.60GHz
 * Version: 8e46048f475a53f99f9e6656e030835b6011f2ca
 * Date: 2018.08.31

Command line:

    time ./q3vm test/test.qvm
    time ./test/test_native

Version History
---------------

 * v1.2 - Debug features enabled (compile with `-DDEBUG_VM`)

 * v1.1 - LCC from ioquake3 added

TODO
----

Known limitations, bugs, missing features:

 * The Quake III Arena JIT compiler (e.g. for x86) is not added.
 * Some 16 bit int operations won't compile with LCC (op code not supported).
 * `DEBUG_VM` builds are not thread-safe.

LICENSE
-------

See COPYING.txt for details on the license. Basically the Quake III Arena
GPL 2 source code license has been inherited.

Be aware that LCC has its own non-commercial license which is described in
lcc/COPYRIGHT.

Further information
-------------------

 * http://fabiensanglard.net/quake3/qvm.php
 * http://users.suse.com/~lnussel/talks/fosdem_talk_2013_q3.pdf

Credits
=======

This project is based on the Quake 3 and ioquake3 source:

 * https://github.com/id-Software/Quake-III-Arena (id Software)
 * https://github.com/ioquake/ioq3

Computed gotos are used:

 * https://eli.thegreenplace.net/2012/07/12/computed-goto-for-efficient-dispatch-tables
 
