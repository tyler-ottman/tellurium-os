#ifndef IDT_H
#define IDT_H

#include <stdint.h>

typedef struct {
    uint16_t size;
    uint64_t offset;
}__attribute__((packed)) IDT_Descriptor;

typedef struct {
    uint16_t offset;
    
}__attribute__((packed)) IDT_Entry;

#endif // IDT_H