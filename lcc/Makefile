# Makefile to build LCC
# This is the LCC from ioquake3
# LCC is in etc/

default: all
all: lcc

lcc:
	@$(MAKE) -C cpp
	@$(MAKE) -C rcc
	@$(MAKE) -C etc

clean:
	@echo 'Cleanup LCC...'
	@$(MAKE) -C cpp clean
	@$(MAKE) -C rcc clean
	@$(MAKE) -C etc clean

.FORCE:

.PHONY: all lcc .FORCE


