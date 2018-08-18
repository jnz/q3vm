Q3VM
====

Jan Zwiener, 2018
Mail: jan@zwiener.org

Standalone interpreter for Quake 3 Virtual machine bytecode files (.qvm files).

Based on Quake 3 and Triseism:

    - https://github.com/id-Software/Quake-III-Arena
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
    
    cd example
    make.bat

Linux:

    Not done yet, sorry

Run the bytecode firmware
-------------------------

    > q3vm bytecode.qvm

