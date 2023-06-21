override CXX = x86_64-elf-g++

override SRC_DIR = ./src
override LIBNAME = ulibc
override INCLUDE = -I../ -I$(SRC_DIR)

override CFLAGS += $(INCLUDE)				\
	-ffreestanding							\
	-fno-exceptions							\
	-fno-rtti								\
	-m64									\
	-nostdlib								\
	-Wall									\
	-mgeneral-regs-only						\
	-mcmodel=large

override CPPFILES := $(shell find . -type f -name '*.cpp')
override CFILES := $(shell find . -type f -name '*.c')
override ASFILES := $(shell find . -type f -name '*.S')
override NASMFILES := $(shell find . -type f -name '*.asm')
override OBJ := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(ASFILES:.S=.o) $(NASMFILES:.asm=.o)

all: $(LIBNAME)

$(LIBNAME): $(OBJ)
	ar r $(LIBNAME).a $(OBJ)

%.o: %.s
	$(CXX) $(INCLUDE) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CXX) $(INCLUDE) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(INCLUDE) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ) $(LIBNAME).a
