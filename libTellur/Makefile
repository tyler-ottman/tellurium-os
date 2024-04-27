override GXX = x86_64-elf-g++

override INCLUDE = -I../

override CFLAGS += $(INCLUDE)				\
	-ffreestanding							\
	-fno-exceptions							\
	-m64									\
	-nostdlib								\
	-Wall									\
	-mgeneral-regs-only						\
	-mcmodel=large							\
	-O2

override CPPFILES := $(shell find . -type f -name '*.cpp')
override OBJ := $(CPPFILES:.cpp=.o)

all: $(OBJ)

%.o: %.cpp
	$(GXX) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ)
