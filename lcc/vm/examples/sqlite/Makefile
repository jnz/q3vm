# Makefile for the sqlite shell running in q3vm.
#
#   make SQLITE=DIR    shell.qvm, the shell as a VM image
#
# DIR holds the sqlite amalgamation and shell from sqlite.org: sqlite3.c, sqlite3.h,
# shell.c and qrf.h. They are not in this repository. The image needs the 64-bit tools
# (lcc-run and vmrun64 built with WIDTH=64) and a heap of HEAP bytes, see the README.
#
#   make WIDTH=64 -C ../.. lcc-run vmrun64
#   make SQLITE=DIR
#   ../../vmrun64 -heap=200M shell.qvm :memory: < script.sql

SQLITE ?= .
HEAP   ?= 200M
LCC_RUN ?= ../../lcc-run

all: shell.qvm

# the options of sqlite for the VM: no Unix file layer, no threads, no extensions
# (vmglue.c registers the VFS that sqlite needs, see there)
sqlite3_vm.c: defs.h $(SQLITE)/sqlite3.c
	cat defs.h $(SQLITE)/sqlite3.c > $@

shell.qvm: sqlite3_vm.c vmglue.c $(SQLITE)/shell.c
	$(LCC_RUN) -heap=$(HEAP) -I $(SQLITE) -o $@ -s sqlite3_vm.c -s vmglue.c $(SQLITE)/shell.c

clean:
	rm -f sqlite3_vm.c shell.qvm

.PHONY: clean
