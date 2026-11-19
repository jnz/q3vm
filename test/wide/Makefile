# Makefile for the 64-bit integer and double test
#
# Builds wide_test.qvm with LCC and q3asm, and wide_host, which runs every test
# case in the VM and natively and compares the results.
#
# Run 'make test'

SHELL = /bin/bash # required on macOS so that export PATH below works
export PATH := $(abspath ../../bin/linux):$(PATH)

# -ffp-contract=off: no fused multiply-add, so the native results match the VM
HOST_CC     = gcc
HOST_CFLAGS = -std=gnu99 -O2 -ffp-contract=off -Wall -Wextra -I../../src/vm

all: wide_test.qvm wide_host

build/wide_test.asm: wide_test.c
	@mkdir -p build
	lcc -DQ3_VM -S -Wf-target=bytecode -Wf-g -o $@ $<

wide_test.qvm: build/wide_test.asm wide_syscalls.asm
	q3asm -o wide_test build/wide_test wide_syscalls

wide_host: wide_host.c wide_test.c ../../src/vm/vm.c ../../src/vm/vm.h
	$(HOST_CC) $(HOST_CFLAGS) -o $@ wide_host.c ../../src/vm/vm.c -lm

test: wide_test.qvm wide_host
	./wide_host wide_test.qvm

clean:
	rm -rf build wide_test.qvm wide_host

.PHONY: all test clean
