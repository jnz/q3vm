# Makefile for rcc, the LCC compiler proper, running in q3vm.
#
#   rcc.qvm  the compiler (lcc/src) as VM bytecode, built with the native lcc
#   vmrcc    host runner that loads rcc.qvm, serves its file and environment
#            calls and passes the arguments on to rcc's main()
#
# WIDTH selects the 32-bit (default) or the 64-bit target and VM:
#   make WIDTH=32   bytecode target, the compiler uses 32-bit values (LCC_WIDE32)
#   make WIDTH=64   bytecode64 target, the VM is built with Q3VM_INT64
#
# Run 'make test' to compare the VM-hosted compiler with the native one.

SHELL = /bin/bash # required on macOS so that export PATH below works
export PATH := $(abspath ../../bin/linux):$(PATH)

WIDTH ?= 32
ifeq ($(WIDTH),64)
TARGET   = bytecode64
WIDE_DEF =
VM_DEF   = -DQ3VM_INT64
else ifeq ($(WIDTH),32)
TARGET   = bytecode
WIDE_DEF = -DLCC_WIDE32
VM_DEF   =
else
$(error WIDTH must be 32 or 64)
endif

HOST_CC     = gcc
HOST_CFLAGS = -std=gnu99 -O2 -ffp-contract=off -Wall -Wextra -I../../src/vm $(VM_DEF)

# -N: only the headers in include/ are used, never the host system headers.
# A -I directory is relative to the directory of the source file, not to the
# current directory, so the rcc sources in ../src use ../vm/include.
VM_CFLAGS = -S -Wf-target=$(TARGET) -N

RCC_SRCS = $(wildcard ../src/*.c)
RCC_ASMS = $(patsubst ../src/%.c,build/rcc_%.asm,$(RCC_SRCS))
RCC_OBJS = $(patsubst build/%.asm,build/%,$(RCC_ASMS))

# vmmain must be first, the VM starts at the first function of the image
LIB_OBJS = build/vmmain build/vmlib build/bg_lib

all: rcc.qvm vmrcc

build/rcc_%.asm: ../src/%.c $(wildcard ../src/*.h) $(wildcard include/*.h)
	@mkdir -p build
	lcc $(VM_CFLAGS) $(WIDE_DEF) -I../vm/include -o $@ $<

build/vmmain.asm: vmmain.c vmsys.h
	@mkdir -p build
	lcc $(VM_CFLAGS) -Iinclude -o $@ $<

build/vmlib.asm: vmlib.c vmsys.h
	@mkdir -p build
	lcc $(VM_CFLAGS) -Iinclude -o $@ $<

build/bg_lib.asm: ../../test/bg_lib.c ../../test/bg_lib.h
	@mkdir -p build
	lcc $(VM_CFLAGS) -o $@ $<

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
	TARGET=$(TARGET) ./check.sh -I ../vm/include ../src/*.c
	TARGET=$(TARGET) VMRCC=$(abspath vmrcc_checked) ./check.sh -I ../vm/include ../src/*.c

clean:
	rm -rf build rcc.qvm vmrcc vmrcc_checked

.PHONY: all clean test
