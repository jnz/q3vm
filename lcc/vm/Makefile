# Makefile for rcc, the LCC compiler proper, running in q3vm.
#
#   rcc.qvm  the compiler (lcc/src) as VM bytecode, built with the native lcc
#   vmrcc    host runner that loads rcc.qvm, serves its file and environment
#            calls and passes the arguments on to rcc's main()
#
# Run 'make test' to compare the VM-hosted compiler with the native one.

SHELL = /bin/bash # required on macOS so that export PATH below works
export PATH := $(abspath ../../bin/linux):$(PATH)

HOST_CC     = gcc
HOST_CFLAGS = -std=gnu99 -O2 -ffp-contract=off -Wall -Wextra -I../../src/vm

# -N: only the headers in include/ are used, never the host system headers.
# A -I directory is relative to the directory of the source file, not to the
# current directory, so the rcc sources in ../src use ../vm/include.
VM_CFLAGS = -S -Wf-target=bytecode -N

RCC_SRCS = $(wildcard ../src/*.c)
RCC_ASMS = $(patsubst ../src/%.c,build/rcc_%.asm,$(RCC_SRCS))
RCC_OBJS = $(patsubst build/%.asm,build/%,$(RCC_ASMS))

# vmmain must be first, the VM starts at the first function of the image
LIB_OBJS = build/vmmain build/vmlib build/bg_lib

all: rcc.qvm vmrcc

build/rcc_%.asm: ../src/%.c $(wildcard ../src/*.h) $(wildcard include/*.h)
	@mkdir -p build
	lcc $(VM_CFLAGS) -I../vm/include -o $@ $<

build/vmmain.asm: vmmain.c vmsys.h
	@mkdir -p build
	lcc $(VM_CFLAGS) -Iinclude -o $@ $<

build/vmlib.asm: vmlib.c vmsys.h
	@mkdir -p build
	lcc $(VM_CFLAGS) -Iinclude -o $@ $<

build/bg_lib.asm: ../../test/bg_lib.c ../../test/bg_lib.h
	@mkdir -p build
	lcc -S -Wf-target=bytecode -N -o $@ $<

rcc.qvm: $(addsuffix .asm,$(LIB_OBJS)) $(RCC_ASMS) vm_syscalls.asm
	q3asm -o rcc $(LIB_OBJS) $(RCC_OBJS) vm_syscalls

vmrcc: vmrcc.c ../../src/vm/vm.c ../../src/vm/vm.h
	$(HOST_CC) $(HOST_CFLAGS) -o $@ vmrcc.c ../../src/vm/vm.c -lm

# checked runner: a valgrind-like VM, see VM_CHECKED in src/vm/vm.c
vmrcc_checked: vmrcc.c ../../src/vm/vm.c ../../src/vm/vm.h
	$(HOST_CC) $(HOST_CFLAGS) -DVM_CHECKED -o $@ vmrcc.c ../../src/vm/vm.c -lm

# every source of the compiler, compiled by the VM and by the native compiler,
# the output must be the same. The checked VM also reports memory and stack errors.
test: all vmrcc_checked
	./check.sh -I ../vm/include ../src/*.c
	VMRCC=$(abspath vmrcc_checked) ./check.sh -I ../vm/include ../src/*.c

clean:
	rm -rf build rcc.qvm vmrcc vmrcc_checked

.PHONY: all clean test
