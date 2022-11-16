#include <acpi/acpi.h>

extern int breakpoint();

static volatile struct limine_rsdp_request kernel_rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};

static struct RSDP* rsdp = NULL;
static struct MADT* madt = NULL;
static struct RSDT* rsdt = NULL;
static struct XSDT* xsdt = NULL;

size_t core_count = 0;
size_t local_apic_index = 0;
size_t io_apic_index = 0;
uint32_t local_apic_ID[128];
uint32_t io_apic_ID[128];
uint64_t io_apic_addr = 0;
uint64_t lapic_addr = 0;

size_t get_rsdp_size(const struct RSDP* rsdp) {
    return rsdp->revision >= 2 ? RSDP_64 : RSDP_32;
}

size_t get_sdt_entry_size(const struct RSDP* rsdp) {
    return get_rsdp_size(rsdp) == RSDP_64 ? 8 : 4;
}

struct MADT* get_madt() {
    return madt;
}

uint32_t* get_lapic_addr() {
    return (uint32_t*)lapic_addr; 
}

bool is_xsdt(const struct RSDP* rsdp) {
    return rsdp->revision >= 2;
}

bool verify_checksum(const uint8_t* data, size_t num_bytes) {
    uint8_t checksum = 0;
    for (size_t i = 0; i < num_bytes; i++) {
        checksum += data[i];
    }
    return !checksum;
}

void init_apic_info(const struct MADT* madt) {
    for (size_t offset = 0x2c; offset < madt->sdt.length;) {
        struct MADT_record* record = (struct MADT_record*)((uint8_t*)madt + offset);
        int type = record->entry_type;
        
        if (type == ENTRY_LAPIC) {
            struct proc_lapic* lapic = (struct proc_lapic*)record;
            if (lapic->flags & 0x3) {
                core_count++;
                local_apic_ID[local_apic_index++] = lapic->apic_id;
            }
        } else if (type == ENTRY_IO_APIC) {
            struct io_apic* io_apic = (struct io_apic*)record;
            io_apic_addr = io_apic->io_apic_address;
            io_apic_ID[io_apic_index++] = io_apic->io_apic_id;
        } else if (type == ENTRY_INT_OVERRIDE) {
            struct int_src_override* override = (struct int_src_override*)record;
            kprintf("ACPI: ENTRY_INT_OVERRIDE: %x\n", override->global_sys_interrupt);
        }
        
        offset += record->record_length;
    }
}

uint64_t find_lapic_addr(const struct MADT* madt) {
    for (size_t offset = 0x2c; offset < madt->sdt.length;) {
        struct MADT_record* record = (struct MADT_record*)((uint8_t*)madt + offset); 
        if (record->entry_type == LOCAL_APIC_ADDR_OVERRIDE) {
            struct local_apic_addr_override* io_apic = (struct local_apic_addr_override*)record;
            return io_apic->local_apic_addr; 
        }
        
        offset += record->record_length;
    }
    return madt->local_interrupt_ctrl_addr;
}

void* find_sdt(const char* sig) {
    struct SDT* sdt;
    size_t sdt_size = is_xsdt(rsdp) ? xsdt->sdt.length : rsdt->sdt.length;
    size_t num_sdts = (sdt_size - 36) / get_sdt_entry_size(rsdp);

    for (size_t i = 0; i < num_sdts; i++) {
        if (is_xsdt(rsdp)) {
            sdt = (struct SDT*)(*((uint64_t*)xsdt->entries + i));
        } else {
            sdt = (struct SDT*)(uint64_t)((*((uint32_t*)rsdt->entries + i)));
        }
        
        if (!__memcmp(sig, sdt, 4)) {
            return sdt;
        }
    }

    return NULL;
}

void init_acpi() {
    struct limine_rsdp_response* rsdp_response = kernel_rsdp_request.response;

    rsdp = rsdp_response->address;
    rsdt = (struct RSDT*)((uint64_t)rsdp->rsdt_address);
    xsdt = (struct XSDT*)rsdp->xsdt_address;

    if (!verify_checksum((const uint8_t*)rsdp, get_rsdp_size(rsdp))) {
        kerror(INVALID_SDT("RSDP"));
    }

    if (is_xsdt(rsdp) && !verify_checksum((const uint8_t*)xsdt, xsdt->sdt.length)) {
        kerror(INVALID_SDT("XSDT"));
    } else if (!verify_checksum((const uint8_t*)rsdt, rsdt->sdt.length)) {
        kerror(INVALID_SDT("RSDT"));
    }

    madt = find_sdt("APIC");
    if (!madt || !verify_checksum((const uint8_t*)madt, madt->sdt.length)) {
        kerror(INVALID_SDT("MADT"));
    }

    lapic_addr = find_lapic_addr(madt);
    init_apic_info(madt);

    kprintf(LIGHT_GREEN"ACPI: Initialized\n");
}