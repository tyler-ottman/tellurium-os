override CXX = x86_64-elf-gcc

override INCLUDE = -I./

override CFLAGS += $(INCLUDE)				\
	-ffreestanding							\
	-fno-exceptions							\
	-m64									\
	-nostdlib								\
	-Wall									\
	-mgeneral-regs-only						\
	-mcmodel=large

override CFILES := $(shell find . -type f -name '*.c')
override ASFILES := $(shell find . -type f -name '*.S')
override NASMFILES := $(shell find . -type f -name '*.asm')
override OBJ := $(CFILES:.c=.o) $(ASFILES:.S=.o) $(NASMFILES:.asm=.o)

all: clean $(OBJ)

%.o: %.s
	$(GXX) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ)
