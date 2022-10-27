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

size_t get_rsdp_size(const struct RSDP* rsdp) {
    return rsdp->revision >= 2 ? RSDP_64 : RSDP_32;
}

size_t get_sdt_entry_size(const struct RSDP* rsdp) {
    return get_rsdp_size(rsdp) == RSDP_64 ? 8 : 4;
}

void sdt_error(const char* sdt_table) {
    terminal_printf(LIGHT_RED "ACPI: Fatal: Invalid %s Table\n", sdt_table);
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
    rsdt = (struct RSDT*)rsdp->rsdt_address;
    xsdt = (struct XSDT*)rsdp->xsdt_address;

    if (!verify_checksum((const uint8_t*)rsdp, get_rsdp_size(rsdp))) {
        sdt_error("RSDP");
    }

    // If RSDT, verify its checksum. Otherwise verify XSDT checksum
    if (is_xsdt(rsdp) && !verify_checksum((const uint8_t*)xsdt, xsdt->sdt.length)) {
        sdt_error("XSDT");
    } else if (!verify_checksum((const uint8_t*)rsdt, rsdt->sdt.length)) {
        sdt_error("RSDT");
    }

    // Find MADT
    madt = find_sdt("APIC");
    if (!madt) {
        sdt_error("MADT");
    }

    // Get MADT
    // madt = find_madt(sdt, is_xsdt(rsdp));
    terminal_printf(LIGHT_GREEN"ACPI: Initialized\n");
}