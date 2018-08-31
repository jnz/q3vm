# Makefile to build the test.qvm bytecode output file
# Optional: test_native(.exe) is compiled from g_main.c
# to benchmark the performance of the .qvm vs. the native build
# 
# Run 'make'
#
# Jan Zwiener 2018

#We try to detect the OS we are running on, and adjust commands as needed
ifeq ($(OS),Windows_NT)
	CLEANUP = rm -f
	MKDIR = mkdir
	TARGET_NATIVE_EXTENSION=.exe
	export PATH := ../bin/win32:$(PATH)
else
	CLEANUP = rm -f
	MKDIR = mkdir -p
	TARGET_NATIVE_EXTENSION=
	SHELL=/bin/bash # required on macOS so that export PATH below works
	export PATH := ../bin/linux:$(PATH)
endif
TARGET_EXTENSION=.qvm

# Target executable configuration
TARGET_BASE=test
TARGET = $(TARGET_BASE)$(TARGET_EXTENSION)
TARGET_NATIVE = $(TARGET_BASE)_native$(TARGET_NATIVE_EXTENSION)

# LCC VM settings
LCC=lcc
LINK := q3asm
LCCFLAGS = -DQ3_VM -S -Wf-target=bytecode -Wf-g

# Native settings
CFLAGS += -std=c99
CFLAGS += -fdata-sections -ffunction-sections -fno-strict-aliasing
CFLAGS += -fmessage-length=0 -MMD -fno-common -MP -MF"$(@:%.o=%.d)"
CFLAGS += -Wall
CFLAGS += -O2
CC=$(TOOLCHAIN)cc
LINK_NATIVE := $(CC)
LOCAL_LIBRARIES := -lm

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
	$(LCC) $(LCCFLAGS) -o $@ $<

all: $(TARGET) $(TARGET_NATIVE)

default: $(TARGET)

$(TARGET): $(OBJDIR) $(OBJS)
	$(LINK) -f bytecode
	@echo 'Executable created: '$@

# Optional: build g_main.c as native application for benchmarks
$(TARGET_NATIVE): $(OBJDIR)/g_main.o
	$(LINK_NATIVE) -o"$(TARGET_NATIVE)" $< $(LOCAL_LIBRARIES)
	@echo 'Executable created: '$@

$(OBJDIR)/g_main.o: g_main.c
	$(CC) $(CFLAGS) -c -o"$(OBJDIR)/g_main.o" "g_main.c"

clean:
	@echo 'Cleanup...'
	$(CLEANUP) $(OBJDIR)/*.asm
	$(CLEANUP) $(OBJDIR)/*.o
	$(CLEANUP) $(OBJDIR)/*.d
	$(CLEANUP) $(TARGET_BASE).map
	$(CLEANUP) $(TARGET)
	$(CLEANUP) $(TARGET_NATIVE)

post-build:

.FORCE:

.PHONY: all .FORCE

.SECONDARY: post-build

