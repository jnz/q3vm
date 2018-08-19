# Makefile to build the Q3VM interpreter
# 
# Run 'make'
#
# Jan Zwiener 2018

# Custom config (optional)
-include config.mk

#We try to detect the OS we are running on, and adjust commands as needed
ifeq ($(OS),Windows_NT)
	CLEANUP = rm -f
	MKDIR = mkdir
	TARGET_EXTENSION=.exe
else
	CLEANUP = rm -f
	MKDIR = mkdir -p
	TARGET_EXTENSION=
endif
# Target executable configuration
TARGET_BASE=q3vm
TARGET = $(TARGET_BASE)$(TARGET_EXTENSION)

# Compiler settings
CC=$(TOOLCHAIN)gcc
LINK := $(CC)
CFLAGS = -std=c99
CFLAGS += -O2 -ggdb -c
CFLAGS += -flto -fno-builtin -fdata-sections -ffunction-sections
# -c: Compile without linking
# -MMD: to autogenerate dependencies for make
# -MP: These dummy rules work around errors make gives if you remove header files without updating the Makefile to match.
# -MF: When used with the driver options -MD or -MMD, -MF overrides the default dependency output file.
# -fno-common: This has the effect that if the same variable is declared (without extern) in two different compilations, you get a multiple-definition error when you link them
# -fmessage-length=n: If n is zero, then no line-wrapping is done; each error message appears on a single line.
CFLAGS += -fmessage-length=0 -MMD -fno-common -MP -MF"$(@:%.o=%.d)"
CFLAGS += -Wall
# disable some warnings...
CFLAGS += -Wno-implicit-int -Wno-implicit-function-declaration -Wno-unused-result -Wno-return-type -Wno-unused-function
# Header files
INCLUDE_PATH := -I"src"
LINK_FLAGS := -Wl,--gc-sections

# Source folders
SRC_SUBDIRS := ./src

# Add all files from the folders in SRC_SUBDIRS to the build
OBJDIR           := build
SOURCES          = $(foreach dir, $(SRC_SUBDIRS), $(wildcard $(dir)/*.c))
C_SRCS           = $(SOURCES)
VPATH            = $(SRC_SUBDIRS)
OBJ_NAMES        = $(notdir $(C_SRCS))
OBJS             = $(addprefix $(OBJDIR)/,$(OBJ_NAMES:%.c=%.o))
C_DEPS           = $(OBJS:%.o=%.d)
C_INCLUDES       = $(INCLUDE_PATH)
LOCAL_LIBRARIES = -lm

$(OBJDIR)/%.o: %.c
	@echo 'CC: $<'
	@$(CC) $(CFLAGS) -o"$@" "$<"

all: $(TARGET)

$(TARGET): $(OBJDIR) $(OBJS)
	@echo 'CFLAGS: '$(CFLAGS)
	$(LINK) $(LINK_FLAGS) -o"$@" $(OBJS) $(LOCAL_LIBRARIES)
	@echo 'Executable created: '$@

clean:
	@echo 'Cleanup...'
	$(CLEANUP) $(OBJDIR)/*.d
	$(CLEANUP) $(OBJDIR)/*.o
	$(CLEANUP) ./$(TARGET)

test: $(TARGET) example/bytecode.qvm
	time ./q3vm example/bytecode.qvm

# Test
example/bytecode.qvm:
	$(MAKE) -C example

# Make sure that we recompile if a header file was changed
-include $(C_DEPS)

post-build:

.FORCE:

.PHONY: all test example/bytecode.qvm .FORCE

.SECONDARY: post-build

