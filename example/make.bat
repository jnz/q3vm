@echo off

set curpath=%CD%
setlocal
REM Add LCC.exe to path
cd ..\bin\win32
set path=%PATH%;%CD%
cd %curpath%

set cc=lcc -DQ3_VM -S -Wf-target=bytecode -Wf-g %1

cd build

%cc%  ../g_main.c
@if errorlevel 1 goto quit

%cc%  ../bg_lib.c
@if errorlevel 1 goto quit

q3asm -f ../bytecode
mv bytecode.qvm ..
:quit
cd ..

