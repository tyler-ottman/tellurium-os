override CXX = gcc

override OBJ_DIR = ./obj
override SRC_DIR = ./src
override BIN_DIR = ./bin
override INC_DIR = ./include

override LDFILE = ./linker.ld
override INCLUDE = -I $(INC_DIR)
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

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.s
	$(CXX) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CXX) $(CFLAGS) $(INCLUDE) -c $< -o $@

directories: $(OBJ_DIR) $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(SRC_DIR)/*.o
	rm -rf $(OBJ_DIR)
	rm -rf $(BIN_DIR)
