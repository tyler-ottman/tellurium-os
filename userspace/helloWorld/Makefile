override CXX = gcc

override SRC_DIR = ./src
override BIN_DIR = ./bin

override LDFILE = ./linker.ld
override CFLAGS = -ffreestanding -m64 -Wall -nostdlib -mgeneral-regs-only
override LDFLAGS = -T $(LDFILE) -nostartfiles

override CFILES := $(shell find . -type f -name '*.c')
override ASFILES := $(shell find . -type f -name '*.S')
override NASMFILES := $(shell find . -type f -name '*.asm')
override OBJ := $(CFILES:.c=.o) $(ASFILES:.S=.o) $(NASMFILES:.asm=.o)
override HEADER_DEPS := $(CFILES:.c=.d) $(ASFILES:.S=.d)

override APP_NAME := helloWorld

all: directories $(BIN_DIR)/$(APP_NAME)

$(BIN_DIR)/$(APP_NAME): $(OBJ)
	$(CXX) $(LDFLAGS) $(OBJ) -o $(BIN_DIR)/$(APP_NAME).elf $(CFLAGS)

%.o: %.s
	$(CXX) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CXX) $(CFLAGS) -c $< -o $@

directories:  $(BIN_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR) $(OBJ)
