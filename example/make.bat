@echo off

set curpath=%CD%
setlocal
REM Add LCC.exe to path
cd ..\lcc\bin
set path=%PATH%;%CD%
cd %curpath%

set cc=lcc -DQ3_VM -S -Wf-target=bytecode -Wf-g %1

mkdir build
cd build

%cc%  ../g_main.c
@if errorlevel 1 goto quit

%cc%  ../bg_lib.c
@if errorlevel 1 goto quit

q3asm -f ../bytecode
:quit
cd ..

