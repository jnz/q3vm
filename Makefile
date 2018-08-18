# Jan Zwiener 2018

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

C_COMPILER=$(TOOLCHAIN)gcc

CFLAGS = -std=c99
CFLAGS += -O2
CFLAGS += -Wno-implicit-int -Wno-implicit-function-declaration

#CFLAGS += -Wall
#CFLAGS += -Wextra

TARGET_BASE=q3vm
TARGET = $(TARGET_BASE)$(TARGET_EXTENSION)
SRC_FILES=\
	src/main.c \
	src/q3vm.c \
	src/q3vmops.c \
	src/q3vmtrap.c \

INC_DIRS = -I./src

all: q3vm

q3vm:
	@echo 'Building q3vm...'
	$(C_COMPILER) $(CFLAGS) $(INC_DIRS) $(SRC_FILES) -lm -o ./$(TARGET)

clean:
	@echo 'Cleanup...'
	@$(CLEANUP) ./$(TARGET)

