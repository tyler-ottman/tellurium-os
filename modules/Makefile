override CXX = x86_64-elf-gcc

override INCLUDE = -I. -I../

override CFLAGS += $(INCLUDE)				\
	-ffreestanding							\
	-fno-exceptions							\
	-m64									\
	-nostdlib								\
	-Wall									\
	-mgeneral-regs-only						\
	-mcmodel=large

override CFILES := $(shell find . -type f -name '*.c')
override OBJ := $(CFILES:.c=.o)

all: clean $(OBJ)

%.o: %.c
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ)
