#include <devices/serial.h>
#include <arch/terminal.h>
#include <libc/string.h>
#include <stdbool.h>

#define RSDP_64 36
#define RSDP_32 20

struct RSDP {
    uint64_t signature;
    uint8_t checksum;
    uint8_t oemid[6];
    uint8_t revision;
    uint32_t* rsdt_address;
    uint32_t length;
    uint64_t* xsdt_address;
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
} __attribute__ ((packed));

struct RSDT {
    struct SDT sdt;
    char* entries[];
} __attribute__ ((packed));

struct XSDT {
    struct SDT sdt;
    char* entries[];
} __attribute__ ((packed));

#define ENTRY_LAPIC         0
#define ENTRY_IO_APIC       1
#define ENTRY_INT_OVERRIDE  2

struct MADT {
    struct SDT sdt;
    uint32_t local_interrupt_ctrl_addr;
    uint32_t flags;
    uint8_t* entries;
} __attribute__ ((packed));

struct MADT_record {
    uint8_t entry_type;
    uint8_t record_length;
} __attribute__ ((packed));

struct proc_lapic {
    struct MADT_record metadata;
    uint8_t acpi_proc_uid;
    uint8_t apic_id;
    uint32_t flags;
} __attribute__ ((packed));

struct io_apic {
    struct MADT_record metadata;
    uint8_t io_apic_id;
    uint8_t reserved;
    uint32_t* io_apic_address;
    uint32_t global_sys_interrupt_base;
} __attribute__ ((packed));

struct int_src_override {
    struct MADT_record metadata;
    uint8_t bus;
    uint8_t source;
    uint32_t global_sys_interrupt;
    uint16_t flags;
} __attribute__ ((packed));

size_t get_sdt_entry_size(const struct RSDP* rsdp);
size_t get_rsdp_size(const struct RSDP* rsdp);

bool is_xsdt(const struct RSDP* rsdp);
void sdt_error(const char* sdt_table);
bool verify_checksum(const uint8_t* data, size_t num_bytes);

void init_apics(const struct MADT* madt);
void* find_sdt(const char* sig);
void init_acpi(void);