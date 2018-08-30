# Makefile to build the test.qvm bytecode output file
# 
# Run 'make'
#
# Jan Zwiener 2018

#We try to detect the OS we are running on, and adjust commands as needed
ifeq ($(OS),Windows_NT)
	CLEANUP = rm -f
	MKDIR = mkdir
	TOOLCHAIN=
	export PATH := ../bin/win32:$(PATH)
else
	TOOLCHAIN=
	CLEANUP = rm -f
	MKDIR = mkdir -p
	SHELL=/bin/bash # required on macOS so that export PATH below works
	export PATH := ../bin/linux:$(PATH)
endif
TARGET_EXTENSION=.qvm

# Target executable configuration
TARGET_BASE=test
TARGET = $(TARGET_BASE)$(TARGET_EXTENSION)

# Compiler settings
CC=$(TOOLCHAIN)lcc
LINK := $(TOOLCHAIN)q3asm
CFLAGS = -DQ3_VM -S -Wf-target=bytecode -Wf-g

# Source folders
SRC_SUBDIRS := .

# Add all files from the folders in SRC_SUBDIRS to the build
OBJDIR           := build
SOURCES          = $(foreach dir, $(SRC_SUBDIRS), $(wildcard $(dir)/*.c))
C_SRCS           = $(SOURCES)
VPATH            = $(SRC_SUBDIRS)
OBJ_NAMES        = $(notdir $(C_SRCS))
OBJS             = $(addprefix $(OBJDIR)/,$(OBJ_NAMES:%.c=%.asm))

$(OBJDIR)/%.asm: %.c
	@echo 'LCC: $<'
	@$(CC) $(CFLAGS) -o $@ $<

all: $(TARGET)

$(TARGET): $(OBJDIR) $(OBJS)
	@echo 'CFLAGS: '$(CFLAGS)
	$(LINK) -f bytecode
	@echo 'Executable created: '$@

clean:
	@echo 'Cleanup...'
	$(CLEANUP) $(OBJDIR)/*.asm
	$(CLEANUP) $(TARGET_BASE).map
	$(CLEANUP) $(TARGET)

post-build:

.FORCE:

.PHONY: all .FORCE

.SECONDARY: post-build

