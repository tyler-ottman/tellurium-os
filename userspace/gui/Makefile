override CXX = x86_64-elf-g++

override SRC_DIR = ./src
override BIN_DIR = ./bin

override LDFILE = ./linker.ld
override CFLAGS += -I$(SRC_DIR)				\
	-I $(SRC_DIR)							\
	-ffreestanding							\
	-fno-exceptions							\
	-fno-rtti								\
	-m64									\
	-nostdlib								\
	-Wall									\
	-mgeneral-regs-only						\
	-mcmodel=large

override LDFLAGS = -T $(LDFILE) 			\
	-nostdlib								\
	-m elf_x86_64

override CPPFILES := $(shell find . -type f -name '*.cpp')
override CFILES := $(shell find . -type f -name '*.c')
override ASFILES := $(shell find . -type f -name '*.S')
override NASMFILES := $(shell find . -type f -name '*.asm')
override OBJ := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(ASFILES:.S=.o) $(NASMFILES:.asm=.o)
override HEADER_DEPS := $(CPPFILES:.cpp=.d) $(CFILES:.c=.d) $(ASFILES:.S=.d)

override APP_NAME := gui.elf

all: directories $(BIN_DIR)/$(APP_NAME)

# $(BIN_DIR)/$(APP_NAME): $(OBJ)
# 	$(CXX) $(LDFLAGS) $(OBJ) -o $(BIN_DIR)/$(APP_NAME).elf $(CFLAGS)

$(BIN_DIR)/$(APP_NAME): $(OBJ)
	x86_64-elf-ld $(OBJ) $(LDFLAGS) -o $@

-include $(HEADER_DEPS)

%.o: %.s
	$(CXX) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CXX) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

directories:  $(BIN_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR) $(OBJ)
