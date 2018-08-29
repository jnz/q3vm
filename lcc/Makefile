# Makefile to build LCC

default: all
all: lcc

lcc:
	$(MAKE) -C cpp
	$(MAKE) -C rcc
	$(MAKE) -C etc

clean:
	$(MAKE) -C cpp clean
	$(MAKE) -C rcc clean
	$(MAKE) -C etc clean

.FORCE:

.PHONY: all lcc .FORCE


