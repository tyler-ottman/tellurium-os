#include <acpi/acpi.h>

extern int breakpoint();

static volatile struct limine_rsdp_request kernel_rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};

bool is_xsdt(const struct RSDP* rsdp) {
    return rsdp->revision >= 2;
}

bool verify_checksum(const struct RSDP* rsdp) {
    size_t num_bytes = is_xsdt(rsdp) ? 36 : 20;

    uint8_t checksum = 0;
    for (size_t i = 0; i < num_bytes; i++)
        checksum += ((uint8_t*) rsdp)[i];

    return checksum == 0;
}

void init_acpi() {
    struct limine_rsdp_response* rsdp_response = kernel_rsdp_request.response;
    struct RSDP* rsdp = rsdp_response->address;

    if (!verify_checksum(rsdp)) {
        terminal_printf(LIGHT_RED "ACPI: fatal: invalid RSDP table\n");
    }

    uint64_t* system_decriptor_table;
    if (is_xsdt(rsdp)) {
        system_decriptor_table = (uint64_t*)rsdp->xsdt_address;
        terminal_printf("ACPI: XSDT detected.\n");
    } else {
        system_decriptor_table = (uint64_t*)((uint64_t)rsdp->rsdt_address);
        terminal_printf("ACPI: RSDT detected.\n");
    }

    terminal_printf("ACPI: sdt addr: %x\n", system_decriptor_table);
    terminal_printf(LIGHT_GREEN"ACPI: Initialized\n");
}