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
VMLIB_DEF = 
else ifeq ($(WIDTH),32)
TARGET   = bytecode
WIDE_DEF = -DLCC_WIDE32
VM_DEF   =
VMLIB_DEF = -DLCC_WIDE32
else
$(error WIDTH must be 32 or 64)
endif

# the objects depend on WIDTH, which make does not see: a change of width
# removes them first
$(shell mkdir -p build; [ "$$(cat build/.width 2>/dev/null)" = "$(WIDTH)" ] || (rm -rf build/rcc_* build/vm*.asm build/bg_lib.asm rcc.qvm driver.qvm vmrcc vmrcc_checked; echo $(WIDTH) > build/.width))

HOST_CC     = gcc
HOST_CFLAGS = -std=gnu99 -O2 -ffp-contract=off -Wall -Wextra -I../../src/vm $(VM_DEF) -DVM_LOG_STDERR

# -N: only the headers in include/ are used, never the host system headers.
# A -I directory is relative to the directory of the source file, not to the
# current directory, so the rcc sources in ../src use ../vm/include.
VM_CFLAGS = -S -Wf-target=$(TARGET) -N $(WIDE_DEF)

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
	lcc $(VM_CFLAGS) $(VMLIB_DEF) -Iinclude -o $@ $<

# the driver needs almost no heap, and each VM has its heap in its data, so its
# runtime library has a small one (the tools keep the 6 MB heap of build/vmlib)
build/vmlib_small.asm: vmlib.c vmsys.h
	@mkdir -p build
	lcc $(VM_CFLAGS) $(VMLIB_DEF) -DVM_HEAP_SIZE=262144 -Iinclude -o $@ $<

build/bg_lib.asm: ../../test/bg_lib.c ../../test/bg_lib.h
	@mkdir -p build
	lcc $(VM_CFLAGS) -o $@ $<

rcc.qvm: $(addsuffix .asm,$(LIB_OBJS)) $(RCC_ASMS) vm_syscalls.asm
	q3asm -o rcc $(LIB_OBJS) $(RCC_OBJS) vm_syscalls

vmrcc: vmrcc.c vmhost.c vmhost.h vfs.c vfs.h ../../src/vm/vm.c ../../src/vm/vm.h
	$(HOST_CC) $(HOST_CFLAGS) -o $@ vmrcc.c vmhost.c vfs.c ../../src/vm/vm.c -lm

# checked runner: a valgrind-like VM, see VM_CHECKED in src/vm/vm.c
vmrcc_checked: vmrcc.c vmhost.c vmhost.h vfs.c vfs.h ../../src/vm/vm.c ../../src/vm/vm.h
	$(HOST_CC) $(HOST_CFLAGS) -DVM_CHECKED -o $@ vmrcc.c vmhost.c vfs.c ../../src/vm/vm.c -lm

# every source of the compiler, compiled by the VM and by the native compiler,
# the output must be the same. The checked VM also reports memory and stack errors.
test: all vmrcc_checked runexamples cppcheck selftest
	TARGET=$(TARGET) ./check.sh -I ../vm/include ../src/*.c
	TARGET=$(TARGET) VMRCC=$(abspath vmrcc_checked) ./check.sh -I ../vm/include ../src/*.c

# vmrun64: the host runner with 64-bit integers, for the VM images of the
# preprocessor and the assembler. cppcheck.sh compares q3cpp.qvm with the native q3cpp.
vmrun64: vmrcc.c vmhost.c vmhost.h vfs.c vfs.h ../../src/vm/vm.c ../../src/vm/vm.h
	$(HOST_CC) $(HOST_CFLAGS) -DQ3VM_INT64 -o $@ vmrcc.c vmhost.c vfs.c ../../src/vm/vm.c -lm

cppcheck: q3cpp.qvm vmrun64
	./cppcheck.sh $(abspath ../src)/*.c $(abspath examples)/*.c

# driver.qvm: the steps of lcc-run, as a VM image (driver.c). The VM tools build it:
# the preprocessor, the compiler and the assembler run in vmrun64, with the files of
# the directory. DRIVER_CPP_DEFINES is what lcc-run passes to the preprocessor.
DRIVER_CPP_DEFINES = -D__STDC__=1 -D__STRICT_ANSI__ -D__signed__=signed -DQ3_VM -D__LCC__ \
	-DLCC_RUN_WIDTH=$(WIDTH)
ifeq ($(WIDTH),32)
DRIVER_CPP_DEFINES += -DLCC_WIDE32
endif

driver.qvm: driver.c vmsys.h rcc.qvm q3cpp.qvm q3asm.qvm vmrun64 build/vmmain.asm \
	build/vmlib_small.asm build/bg_lib.asm vm_syscalls.asm
	./vmrun64 q3cpp.qvm $(DRIVER_CPP_DEFINES) -Iinclude driver.c driver.i
	./vmrun64 rcc.qvm -target=$(TARGET) driver.i driver.asm
	./vmrun64 q3asm.qvm -o driver build/vmmain build/vmlib_small build/bg_lib driver vm_syscalls

# selftest: lcc-run compiles driver.c with the VM tools, and the driver it makes
# compiles and runs examples/hello.c. The output must be that of hello.c.
selftest: all lcc-run
	./lcc-run driver.c examples/hello.c alpha beta > selftest.out 2>/dev/null
	diff selftest.out examples/hello.expected > /dev/null && echo "ok   selftest: driver.c compiled and run by lcc-run" || { echo "FAIL selftest"; exit 1; }

# lcc-run: one executable with the driver, the compiler, the runtime, the preprocessor,
# the assembler and the include files inside it (see lcc-run.c and payload.py)
LCC_RUN_HEADERS = $(wildcard include/*.h)
LCC_RUN_PAYLOAD = driver.qvm=driver.qvm q3cpp.qvm=q3cpp.qvm q3asm.qvm=q3asm.qvm \
	rcc.qvm=rcc.qvm lib/vmmain.asm=build/vmmain.asm lib/vmlib.asm=build/vmlib.asm \
	lib/bg_lib.asm=build/bg_lib.asm lib/vm_syscalls.asm=vm_syscalls.asm \
	$(foreach h,$(LCC_RUN_HEADERS),$(h)=$(h))

payload.c: driver.qvm rcc.qvm build/vmmain.asm build/vmlib.asm build/bg_lib.asm \
	vm_syscalls.asm $(LCC_RUN_HEADERS) q3cpp.qvm q3asm.qvm payload.py
	python3 -I payload.py payload.c $(LCC_RUN_PAYLOAD)

lcc-run: lcc-run.c vmhost.c vmhost.h vfs.c vfs.h payload.c payload.h ../../src/vm/vm.c ../../src/vm/vm.h
	$(HOST_CC) $(HOST_CFLAGS) -DQ3VM_INT64 -o $@ lcc-run.c vmhost.c vfs.c payload.c ../../src/vm/vm.c -lm

clean:
	rm -rf build64 q3asm.qvm q3cpp.qvm
	rm -f payload.c lcc-run selftest.out driver.i driver.asm driver.qvm driver
	rm -rf build rcc.qvm vmrcc vmrcc_checked vmrun64

.PHONY: all clean test selftest

runexamples: all lcc-run
	@for f in examples/*.c; do \
	    b=$${f%.c}; \
	    ./lcc-run $$f alpha beta > $$b.out 2>/dev/null && diff -q $$b.out $$b.expected >/dev/null && echo "ok   $$f" || { echo "FAIL $$f"; exit 1; }; \
	done

# q3asm.qvm: the assembler as a VM image. It uses 64-bit integers, so it is built
# for the bytecode64 target whatever WIDTH is, with a runtime library of its own
# (build64). The VM that runs it must be built with Q3VM_INT64, as lcc-run is.
Q3ASM_INCLUDE = ../lcc/vm/include # relative to the source directory, ../../q3asm

build64/vmmain.asm: vmmain.c vmsys.h
	@mkdir -p build64
	lcc -S -Wf-target=bytecode64 -N -Iinclude -o $@ $<

build64/vmlib.asm: vmlib.c vmsys.h
	@mkdir -p build64
	lcc -S -Wf-target=bytecode64 -N -Iinclude -o $@ $<

build64/bg_lib.asm: ../../test/bg_lib.c ../../test/bg_lib.h
	@mkdir -p build64
	lcc -S -Wf-target=bytecode64 -N -o $@ $<

build64/q3asm.asm: ../../q3asm/q3asm.c ../../q3asm/opstrings.h ../../q3asm/cmdlib.h \
	../../q3asm/q_platform.h $(wildcard include/*.h)
	@mkdir -p build64
	lcc -S -Wf-target=bytecode64 -N -I$(Q3ASM_INCLUDE) -I../../q3asm -o $@ ../../q3asm/q3asm.c

build64/cmdlib.asm: ../../q3asm/cmdlib.c ../../q3asm/cmdlib.h $(wildcard include/*.h)
	@mkdir -p build64
	lcc -S -Wf-target=bytecode64 -N -I$(Q3ASM_INCLUDE) -I../../q3asm -o $@ ../../q3asm/cmdlib.c

q3asm.qvm: build64/vmmain.asm build64/vmlib.asm build64/bg_lib.asm build64/q3asm.asm \
	build64/cmdlib.asm vm_syscalls.asm
	../../bin/linux/q3asm -o q3asm build64/vmmain build64/vmlib build64/bg_lib \
		build64/q3asm build64/cmdlib vm_syscalls

# q3cpp.qvm: the preprocessor as a VM image, from the sources in ../cpp. vmfd.c
# gives it the file descriptor calls on the host's files.
CPP_SRCS = $(notdir $(wildcard ../cpp/*.c))

build64/vmfd.asm: vmfd.c vmsys.h $(wildcard include/*.h)
	@mkdir -p build64
	lcc -S -Wf-target=bytecode64 -N -Iinclude -o $@ $<

build64/cpp_%.asm: ../cpp/%.c $(wildcard ../cpp/*.h) $(wildcard include/*.h)
	@mkdir -p build64
	lcc -S -Wf-target=bytecode64 -N -DQ3_VM -I../vm/include -o $@ $<

CPP_ASMS = $(patsubst %.c,build64/cpp_%.asm,$(CPP_SRCS))
CPP_OBJS = $(patsubst build64/%.asm,build64/%,$(CPP_ASMS))

q3cpp.qvm: build64/vmmain.asm build64/vmlib.asm build64/bg_lib.asm build64/vmfd.asm \
	$(CPP_ASMS) vm_syscalls.asm
	../../bin/linux/q3asm -o q3cpp build64/vmmain build64/vmlib build64/bg_lib \
		build64/vmfd $(CPP_OBJS) vm_syscalls

