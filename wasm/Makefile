# Makefile for q3vm in WebAssembly, built with Emscripten (emcc).
#
#   make            web/vm.js and web/vm.wasm for the 32-bit VM (default)
#   make WIDTH=64   web/vm.js and web/vm.wasm built with Q3VM_INT64
#   make node       node/vm.js and node/vm.wasm, for running from Node.js
#
# Open web/index.html through a web server (browsers do not load wasm from
# file:// URLs). The module has no file or system call support: an image that
# calls a system call stops the module.

WIDTH ?= 32
ifeq ($(WIDTH),64)
VM_DEF = -DQ3VM_INT64
else
VM_DEF =
endif

EMFLAGS = -O2 -I../src/vm $(VM_DEF) \
	-sMODULARIZE=1 -sEXPORT_NAME=createQ3vm \
	-sEXPORTED_FUNCTIONS=_alloc_bytes,_load,_call,_call64 \
	-sEXPORTED_RUNTIME_METHODS=HEAPU8 -sALLOW_MEMORY_GROWTH=1

SOURCES = wasm_host.c ../src/vm/vm.c ../src/vm/vm.h

all: web/vm.js

web/vm.js: $(SOURCES)
	@mkdir -p web
	emcc $(EMFLAGS) -sENVIRONMENT=web wasm_host.c ../src/vm/vm.c -o $@

node: $(SOURCES)
	@mkdir -p node
	emcc $(EMFLAGS) -sENVIRONMENT=node wasm_host.c ../src/vm/vm.c -o node/vm.js

clean:
	rm -rf web node

.PHONY: all node clean
