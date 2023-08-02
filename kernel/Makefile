# This is the name that our final kernel executable will have.
# Change as needed.
override KERNEL := kernel.elf

# Where freestanding libc function reside
override FLIBC_DIR := ../flibc

# Convenience macro to reliably declare overridable command variables.
define DEFAULT_VAR =
    ifeq ($(origin $1),default)
        override $(1) := $(2)
    endif
    ifeq ($(origin $1),undefined)
        override $(1) := $(2)
    endif
endef
 
# It is highly recommended to use a custom built cross toolchain to build a kernel.
# We are only using "cc" as a placeholder here. It may work by using
# the host system's toolchain, but this is not guaranteed.
# $(eval $(call DEFAULT_VAR,CC,cc))
override CC := x86_64-elf-gcc
 
# Same thing for "ld" (the linker).
$(eval $(call DEFAULT_VAR,LD,ld))
 
# User controllable CFLAGS.
CFLAGS ?= -O2 -g -Wall -Wextra -Wpedantic -pipe
 
# User controllable preprocessor flags. We set none by default.
CPPFLAGS ?=
 
# User controllable nasm flags.
NASMFLAGS ?= -F dwarf -g

override INCLUDE = -I. -I../ -I$(FLIBC_DIR)

# Internal linker flags that should not be changed by the user.
override LDFLAGS ?=         \
    -nostdlib               \
    -static                 \
    -z max-page-size=0x1000 \
    -T linker.ld

# Internal C flags that should not be changed by the user.
override CFLAGS +=              \
	$(INCLUDE)                  \
	-std=c11                    \
	-ffreestanding              \
	-fstack-protector-strong    \
	-fno-pie                    \
	-fno-pic                    \
	-m64                        \
	-march=x86-64               \
	-mabi=sysv                  \
	-mno-80387                  \
	-mno-mmx                    \
	-mno-sse                    \
	-mno-sse2                   \
	-mno-red-zone               \
	-mcmodel=kernel             \
	-MMD
 
# User controllable linker flags. We set none by default.
override LDFLAGS += 

# Internal nasm flags that should not be changed by the user.
override NASMFLAGS += \
    -f elf64
 
# Use find to glob all *.c, *.S, and *.asm files in the directory and extract the object names.
override CFILES := $(shell find . $(FLIBC_DIR) -type f -name '*.c')
override ASFILES := $(shell find . -type f -name '*.S')
override NASMFILES := $(shell find . -type f -name '*.asm')
override OBJ := $(CFILES:.c=.o) $(ASFILES:.S=.o) $(NASMFILES:.asm=.o)
override HEADER_DEPS := $(CFILES:.c=.d) $(ASFILES:.S=.d)
 
# Default target.
all: $(KERNEL)
 
# Link rules for the final kernel executable.
$(KERNEL): $(OBJ)
	$(MAKE) -C $(FLIBC_DIR)
	x86_64-elf-ld $(OBJ) $(LDFLAGS) -o $@
 
# Include header dependencies.
-include $(HEADER_DEPS)
 
# Compilation rules for *.c files.
%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
 
# Compilation rules for *.S files.
%.o: %.S
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
 
# Compilation rules for *.asm (nasm) files.
%.o: %.asm
	nasm $(NASMFLAGS) $< -o $@
 
# Remove object files and the final executable.
.PHONY: clean
clean:
	$(MAKE) -C $(FLIBC_DIR) clean
	rm -rf $(KERNEL) $(OBJ) $(HEADER_DEPS) *.log
