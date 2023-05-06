#include <arch/syscalls.h>
#include <memory/vmm.h>

#define O_WRONLY                    0x01
#define O_RDONLY                    0x02
#define O_RDWR                      0x03

int syscall_open(const char *path, int flags) {
    
}