#include <devices/serial.h>
#include <arch/terminal.h>
#include <stdbool.h>

struct RSDP {
    uint64_t signature;
    uint8_t checksum;
    uint8_t oemid[6];
    uint8_t revision;
    uint32_t rsdt_address;
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__ ((packed));

struct SDT {
    uint32_t signature;
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    uint8_t oemid[6];
    uint64_t oemind_table_id;
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
    void* entry;
};

bool is_xsdt(const struct RSDP* rsdp);
bool verify_checksum(const struct RSDP* rsdp);
void init_acpi(void);