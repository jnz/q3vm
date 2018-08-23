      ___   _______     ____  __
     / _ \ |___ /\ \   / /  \/  |
    | | | |  |_ \ \ \ / /| |\/| |
    | |_| |____) | \ V / | |  | |
     \__\_______/   \_/  |_|  |_|


Standalone and embeddable interpreter for Quake 3 Virtual machine bytecode files (.qvm files). A compiler to generate `.qvm` files (LCC) is included. Computed gotos are used to speed up the interpreter if you compile with GCC.

Jan Zwiener, 2018

Mail: jan@zwiener.org

Q3VM
====

Two things are required:

 * The interpreter
 * A bytecode binary .qvm file

Run:

    > q3vm.exe bytecode.qvm

The q3vm.exe standalone interpreter is not required. You can
easily add a single .c file to your project (''vm.c'' and ''vm.h'').
Call ''VM_Create'' and ''VM_Call'' to run the bytecode in your
application:

    vm_t vm;
    VM_Create(&vm, "my test", pointerToByteCodeBuffer, sysCall);
    VM_Call(&vm, 0);
    VM_Free(&vm);
    
The `pointerToByteCodeBuffer` is some memory location where the
bytecode is located. You can e.g. load it from a file and store it
in an uint8_t array. See `main.c` for an example implementation.

The `sysCall` is a callback function that you define so that
the interpreter can call native functions from your code. E.g. a
logging function or some time critical function that you don't want
to implement in the bytecode. Again check `main.c` for an example.


Build VM/interpreter
--------------------

On **Linux**:

    > make

On **Windows**:
Install MinGW64 from and add the bin\ directory it to your path.
So that you have gcc.exe available.

 * http://mingw-w64.org/doku.php/download/mingw-builds

Compile with:

    > mingw32-make

Or use the Visual Studio 2015 project `q3vm.sln` in the `msvc` subfolder.

Build bytecode firmware
-----------------------

**Windows**:

The LCC compiler (lcc.exe) is saved in the ./bin/win32 directory.
You need make (mingw32-make) from the MinGW64 installation in
your path. The compiler in the repository as .exe file.

    cd example
    mingw32-make
    cp bytecode.qvm ..
    cd ..

**Linux**:

Build LCC (and copy the executables to ./bin/linux directory).

    > cd lcc
    > make BUILDDIR=build all
    > cd build
    > cp lcc ../../bin/linux/lcc
    > cp cpp ../../bin/linux/q3cpp
    > cp rcc ../../bin/linux/q3rcc
    > cd ../..

Build q3asm

    > cd q3asm
    > make
    > cp q3asm ../bin/linux/
    > cd ..

Build the example bytecode:

    > cd example
    > make
    > cp bytecode.qvm ..
    > cd ..

Run the bytecode firmware
-------------------------

    > q3vm bytecode.qvm

Credits
=======

This project is based on the Quake 3 and ioquake3 source:

 * https://github.com/id-Software/Quake-III-Arena (id Software)
 * https://github.com/ioquake/ioq3

Computed gotos are used:

 * https://eli.thegreenplace.net/2012/07/12/computed-goto-for-efficient-dispatch-tables
 
