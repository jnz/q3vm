# Makefile to build LCC
# 
# Run 'make'
#
# Jan Zwiener 2018-2020

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
OBJDIR           := ../build
# Target executable configuration
TARGET_BASE=$(OBJDIR)/rcc
TARGET = $(TARGET_BASE)$(TARGET_EXTENSION)

# Linker settings
LINK_FLAGS :=

# Compiler settings
CC=$(TOOLCHAIN)gcc
LINK := $(CC)
CFLAGS += -fno-strict-aliasing
# -MMD: to autogenerate dependencies for make
# -MP: These dummy rules work around errors make gives if you remove header files without updating the Makefile to match.
# -MF: When used with the driver options -MD or -MMD, -MF overrides the default dependency output file.
# -fno-common: This has the effect that if the same variable is declared (without extern) in two different compilations, you get a multiple-definition error when you link them
# -fmessage-length=n: If n is zero, then no line-wrapping is done; each error message appears on a single line.
CFLAGS += -fmessage-length=0 -MMD -fno-common -MP -MF"$(@:%.o=%.d)"
CFLAGS += -Wall -Wno-misleading-indentation
# CFLAGS += -Og -ggdb -fno-omit-frame-pointer
CFLAGS += -O0

# disable some warnings...
# Header files
INCLUDE_PATH := -I"./"

# Source folders
SRC_SUBDIRS := ../src

# Add all files from the folders in SRC_SUBDIRS to the build
SOURCES          = $(foreach dir, $(SRC_SUBDIRS), $(wildcard $(dir)/*.c))
C_SRCS           = $(SOURCES)
VPATH            = $(SRC_SUBDIRS)
OBJ_NAMES        = $(notdir $(C_SRCS))
OBJS             = $(addprefix $(OBJDIR)/,$(OBJ_NAMES:%.c=%.o))
C_DEPS           = $(OBJS:%.o=%.d)
C_INCLUDES       = $(INCLUDE_PATH)
LOCAL_LIBRARIES = -lm

# flag -c: Compile without linking
$(OBJDIR)/%.o: %.c
	@echo 'CC: $<'
	@$(CC) $(CFLAGS) $(C_INCLUDES) -c -o"$@" "$<"

default: $(TARGET)

all: $(TARGET)

# requires dagcheck.c
# dagcheck.c is created by lburg dagcheck.md dagcheck.c

$(TARGET): $(OBJDIR) $(OBJS)
	$(LINK) $(LINK_FLAGS) -o"$@" $(OBJS) $(LOCAL_LIBRARIES)
	@echo 'Executable created: '$@

clean:
	$(CLEANUP) $(OBJDIR)/*.d
	$(CLEANUP) $(OBJDIR)/*.o
	$(CLEANUP) $(TARGET)

# Make sure that we recompile if a header file was changed
-include $(C_DEPS)

post-build:

.FORCE:

.PHONY: all .FORCE

.SECONDARY: post-build

