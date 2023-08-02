override CXX = x86_64-elf-gcc
override GXX = x86_64-elf-g++

override SRC_DIR = ./src
override BIN_DIR = ./bin

override TOP_DIR = ../..
override FLIBC_DIR = $(TOP_DIR)/flibc
override LIBGUI_DIR = $(TOP_DIR)/libGUI
override LIBTELLUR_DIR = $(TOP_DIR)/libTellur

override INCLUDE = -I$(SRC_DIR) -I$(TOP_DIR) -I$(FLIBC_DIR)

override CFLAGS += $(INCLUDE)				\
	-I $(SRC_DIR)							\
	-ffreestanding							\
	-fno-exceptions							\
	-m64									\
	-fno-rtti								\
	-nostdlib								\
	-Wall									\
	-mgeneral-regs-only						\
	-mcmodel=large

override LDFILE = ./linker.ld
override LDFLAGS = -T $(LDFILE) 			\
	-nostdlib								\
	-m elf_x86_64

override CPPFILES := $(shell find . $(LIBGUI_DIR) $(LIBTELLUR_DIR) -type f -name '*.cpp')
override CFILES := $(shell find $(FLIBC_DIR) -type f -name '*.c')
override OBJ := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o)
override HEADER_DEPS := $(CPPFILES:.cpp=.d) $(CFILES:.c=.d)

override APP_NAME := testbench.elf

all: directories $(BIN_DIR)/$(APP_NAME)

$(BIN_DIR)/$(APP_NAME): $(OBJ)
	x86_64-elf-ld $(OBJ) $(LDFLAGS) -o $@

-include $(HEADER_DEPS)

%.o: %.c
	$(GXX) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(GXX) $(CFLAGS) -c $< -o $@

directories:  $(BIN_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	$(MAKE) -C $(FLIBC_DIR) clean
	$(MAKE) -C $(LIBGUI_DIR) clean
	$(MAKE) -C $(LIBTELLUR_DIR) clean
	rm -rf $(BIN_DIR) $(OBJ)
