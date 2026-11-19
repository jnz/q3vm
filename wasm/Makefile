# Makefile for q3vm in WebAssembly, built with Emscripten (emcc).
#
#   make            web/vm.js and web/vm.wasm, for a web page
#   make node       node/vm.js and node/vm.wasm, for running from Node.js
#
# The VM is built with Q3VM_INT64, so it runs both the 32-bit and the 64-bit images
# of the toolchain. The system calls are those of lcc/vm/vmhost.c, the files are in
# memory (lcc/vm/vfs.c, without a host backup). See wasm_host.c for the exports.
#
# Open web/ through a web server (browsers do not load wasm from file:// URLs).

EMFLAGS = -O2 -I../src/vm -I../lcc/vm -DQ3VM_INT64 -DVM_LOG_STDERR \
	-sMODULARIZE=1 -sEXPORT_NAME=createQ3vm \
	-sEXPORTED_FUNCTIONS=_vm_init,_alloc_bytes,_free_bytes,_put_file,_file_size,_get_file,_run \
	-sEXPORTED_RUNTIME_METHODS=HEAPU8 -sALLOW_MEMORY_GROWTH=1

SOURCES = wasm_host.c ../lcc/vm/vmhost.c ../lcc/vm/vmhost.h ../lcc/vm/vfs.c ../lcc/vm/vfs.h \
	../src/vm/vm.c ../src/vm/vm.h

all: web/vm.js

web/vm.js: $(SOURCES)
	@mkdir -p web
	emcc $(EMFLAGS) -sENVIRONMENT=web wasm_host.c ../lcc/vm/vmhost.c ../lcc/vm/vfs.c \
		../src/vm/vm.c -o $@

node: node/vm.js

node/vm.js: $(SOURCES)
	@mkdir -p node
	emcc $(EMFLAGS) -sENVIRONMENT=node wasm_host.c ../lcc/vm/vmhost.c ../lcc/vm/vfs.c \
		../src/vm/vm.c -o $@

clean:
	rm -rf web node

.PHONY: all node clean
