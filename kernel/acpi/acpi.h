#ifndef ACPI_H
#define ACPI_H

#include <arch/kterminal.h>
#include <devices/msr.h>
#include <devices/serial.h>
#include <flibc/string.h>
#include <stdbool.h>
#include <stdint.h>

#define MMIO_NUM_DEVS               20
#define IOAPIC_NUM_DEVS             10
#define LAPIC_NUM_DEVS              128

#define ENTRY_LAPIC                 0
#define ENTRY_IO_APIC               1
#define ENTRY_IOAPIC_INT_OVERRIDE   2
#define LOCAL_APIC_ADDR_OVERRIDE    5

#define UNALIGNED_LAPIC "ACPI: Unaligned lapic register\n"
#define INVALID_SDT(SDT) "ACPI: Invalid " SDT " Table\n"

#define RSDP_64 36
#define RSDP_32 20

#define HPET_4KB_PROTECTION         1

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
} __attribute__ ((packed));

struct RSDT {
    struct SDT sdt;
    char* entries[];
} __attribute__ ((packed));

struct XSDT {
    struct SDT sdt;
    char* entries[];
} __attribute__ ((packed));

struct HPET {
    struct SDT sdt;
    uint32_t event_timer_block_id;
    uint8_t address_space_id;
    uint8_t register_bit_width;
    uint8_t register_bit_offset;
    uint8_t reserved;
    uint64_t hpet_address;
    uint8_t hpet_number;
    uint16_t main_counter_min_clock_tick;
    uint8_t page_protection;
} __attribute__ ((packed));

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

typedef struct io_apic {
    struct MADT_record metadata;
    uint8_t io_apic_id;
    uint8_t reserved;
    uint32_t io_apic_address;
    uint32_t global_sys_interrupt_base;
} __attribute__ ((packed)) io_apic_t;

typedef struct ioapic_int_src_override {
    struct MADT_record metadata;
    uint8_t bus;
    uint8_t irq_source;
    uint32_t global_sys_interrupt;
    uint16_t flags;
} __attribute__ ((packed)) ioapic_int_src_override_t;

struct local_apic_addr_override {
    struct MADT_record metadata;
    uint16_t reserved;
    uint32_t local_apic_addr;
} __attribute__ ((packed));

typedef struct mmio_dev {
    uint64_t addr;
    uint64_t size_bytes;
} mmio_dev_t;

typedef struct mmio_dev_info {
    mmio_dev_t devices[MMIO_NUM_DEVS];
    size_t num_devs;
} mmio_dev_info_t;

size_t get_sdt_entry_size(const struct RSDP *rsdp);
size_t get_rsdp_size(const struct RSDP *rsdp);
struct MADT *get_madt(void);

bool acpi_hpet_present(void);
uint32_t *get_lapic_addr(void);
uint32_t *get_hpet_addr(void);
uint32_t *get_lapic_ids(void);
io_apic_t **acpi_get_ioapics(void);
size_t acpi_get_num_ioapics(void);
int acpi_get_gsi_base(uint32_t *ioapic_addr);

size_t get_core_count(void);
mmio_dev_info_t get_dev_info(void);
bool is_xsdt(const struct RSDP *rsdp);
bool verify_checksum(const uint8_t *data, size_t num_bytes);
void init_apic_info(const struct MADT *madt);
int acpi_irq_to_gsi(int irq);
uint64_t find_lapic_addr(const struct MADT *madt);

void *find_sdt(const char *sig);
void init_acpi(void);

#endif // ACPI_H