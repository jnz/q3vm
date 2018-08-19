      ___   _______     ____  __
     / _ \ |___ /\ \   / /  \/  |
    | | | |  |_ \ \ \ / /| |\/| |
    | |_| |____) | \ V / | |  | |
     \__\_______/   \_/  |_|  |_|


Jan Zwiener, 2018
Mail: jan@zwiener.org

Q3VM
====

Standalone interpreter for Quake 3 Virtual machine bytecode files (.qvm files).
Two things are required:
    - The interpreter (q3vm / q3vm.exe)
    - A bytecode binary .qvm file (e.g. bytecode.qvm)

Run:

    > q3vm.exe bytecode.qvm


This is based on the Quake 3 VM, ioquake3 and Triseism:

    - https://github.com/id-Software/Quake-III-Arena
    - https://github.com/ioquake/ioq3
    - http://www.icculus.org/triseism/

Build VM/interpreter
--------------------

On Linux:

    > make

On Windows:
Install MinGW64 from and add the bin\ directory it to your path.
So that you have gcc.exe available.

    - http://mingw-w64.org/doku.php/download/mingw-builds

Compile with:

    > mingw32-make

Build bytecode firmware
-----------------------

Windows:
The LCC compiler (lcc.exe) is saved in the ./bin/win32 directory.

    cd example
    make.bat
    cp bytecode.qvm ..
    cd ..

Linux:

Build LCC

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

