#include <acpi/acpi.h>
#include <sys/misc.h>

static volatile struct limine_rsdp_request kernel_rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};

typedef struct ioapic_version {
    uint8_t apic_version: 8;
    uint8_t reserved0: 8;
    uint8_t max_red_entry: 8;
    uint8_t reserved1: 8;
}__attribute__ ((__packed__)) ioapic_version_t;

static mmio_dev_info_t dev_info;

static struct RSDP *rsdp = NULL;
static struct MADT *madt = NULL;
static struct HPET *hpet = NULL;
static struct RSDT *rsdt = NULL;
static struct XSDT *xsdt = NULL;

size_t core_count = 0;

// List of Local APIC IDs
size_t local_apic_index = 0;
uint32_t local_apic_ID[LAPIC_NUM_DEVS];

// List of I/O APIC structs
size_t io_apic_index = 0;
io_apic_t *ioapics[IOAPIC_NUM_DEVS];

static uint64_t lapic_addr = 0;
static uint64_t hpet_addr = 0;
static bool is_hpet_present = true;

size_t get_rsdp_size(const struct RSDP *rsdp) {
    return rsdp->revision >= 2 ? RSDP_64 : RSDP_32;
}

size_t get_sdt_entry_size(const struct RSDP *rsdp) {
    return get_rsdp_size(rsdp) == RSDP_64 ? 8 : 4;
}

struct MADT *get_madt() {
    return madt;
}

bool acpi_hpet_present() {
    return is_hpet_present;
}

uint32_t *get_lapic_addr() {
    return (uint32_t*)lapic_addr; 
}

uint32_t *get_hpet_addr() {
    return (uint32_t*)hpet_addr;
}

uint32_t *get_lapic_ids() {
    return local_apic_ID;
}

io_apic_t **acpi_get_ioapics() {
    return ioapics;
}

size_t acpi_get_num_ioapics() {
    return io_apic_index;
}

int acpi_get_gsi_base(uint32_t *ioapic_addr) {
    if (!ioapic_addr) {
        return -1;
    }
    for (size_t i = 0; i < io_apic_index; i++) {
        io_apic_t *ioapic = ioapics[i];
        if ((uint64_t)ioapic->io_apic_address == (uint64_t)ioapic_addr) {
            io_apic_t *ioapic = ioapics[i];
            return ioapic->global_sys_interrupt_base;
        }
    }
    return -1;
}

size_t get_core_count() {
    return core_count;
}

mmio_dev_info_t get_dev_info() {
    return dev_info;
}

bool is_xsdt(const struct RSDP *rsdp) {
    return rsdp->revision >= 2;
}

bool verify_checksum(const uint8_t *data, size_t num_bytes) {
    uint8_t checksum = 0;
    for (size_t i = 0; i < num_bytes; i++) {
        checksum += data[i];
    }
    return !checksum;
}

int acpi_irq_to_gsi(int irq) {
    if (!madt) {
        return -1;
    }

    for (size_t offset = 0x2c; offset < madt->sdt.length;) {
        struct MADT_record *record = (struct MADT_record *)((uint8_t *)madt + offset);
        if (record->entry_type == ENTRY_IOAPIC_INT_OVERRIDE) {
            ioapic_int_src_override_t *src = (ioapic_int_src_override_t *)record;
            if (src->irq_source == irq) {
                return src->global_sys_interrupt;
            }
        }
        offset += record->record_length;
    }

    // Legacy IRQ number is gsi
    return irq;
}

void init_apic_info(const struct MADT *madt) {
    for (size_t offset = 0x2c; offset < madt->sdt.length;) {
        struct MADT_record *record = (struct MADT_record *)((uint8_t *)madt + offset);
        int type = record->entry_type;
        
        if (type == ENTRY_LAPIC) {
            struct proc_lapic *lapic = (struct proc_lapic *)record;
            if (lapic->flags & 0x3) {
                core_count++;
                ASSERT(local_apic_index < core_count, 0,
                       "ACPI: cores exceed local APIC IDs list");
                local_apic_ID[local_apic_index++] = lapic->apic_id;
                // kprintf("ACPI: Lapic %x detected\n", lapic->apic_id);
            }
        } else if (type == ENTRY_IO_APIC) {
            io_apic_t *ioapic = (io_apic_t *)record;
            ASSERT(io_apic_index < IOAPIC_NUM_DEVS, 0,
                   "ACPI: I/O APIC devices exceed I/O APIC device list");
            ioapics[io_apic_index++] = ioapic;
        }
        
        offset += record->record_length;
    }
}

uint64_t find_lapic_addr(const struct MADT *madt) {
    for (size_t offset = 0x2c; offset < madt->sdt.length;) {
        struct MADT_record *record = (struct MADT_record *)((uint8_t *)madt + offset); 
        if (record->entry_type == LOCAL_APIC_ADDR_OVERRIDE) {
            struct local_apic_addr_override *lapic64 = (struct local_apic_addr_override *)record;
            return lapic64->local_apic_addr; 
        }
        
        offset += record->record_length;
    }
    return madt->local_interrupt_ctrl_addr;
}

void *find_sdt(const char *sig) {
    struct SDT* sdt;
    size_t sdt_size = is_xsdt(rsdp) ? xsdt->sdt.length : rsdt->sdt.length;
    size_t num_sdts = (sdt_size - 36) / get_sdt_entry_size(rsdp);

    for (size_t i = 0; i < num_sdts; i++) {
        if (is_xsdt(rsdp)) {
            sdt = (struct SDT *)(*((uint64_t *)xsdt->entries + i));
        } else {
            sdt = (struct SDT *)(uint64_t)((*((uint32_t *)rsdt->entries + i)));
        }
        
        if (!__memcmp(sig, sdt, 4)) {
            return sdt;
        }
    }

    return NULL;
}

static void add_mmio_device(mmio_dev_t mmio_dev) {
    dev_info.devices[dev_info.num_devs++] = mmio_dev;
}

static bool init_dev_hpet() {
    hpet = find_sdt("HPET");
    if (!hpet || !verify_checksum((const uint8_t *)hpet, hpet->sdt.length)) {
        return false;
    }
    
    hpet->page_protection = HPET_4KB_PROTECTION;
    hpet_addr = hpet->hpet_address;

    mmio_dev_t dev_lapic = {
        .addr = hpet->hpet_address,
        .size_bytes = 4096
    };

    add_mmio_device(dev_lapic);
    return true;
}

static void init_devs_ioapic() {
    for (size_t offset = 0x2c; offset < madt->sdt.length;) {
        struct MADT_record *record = (struct MADT_record *)((uint8_t *)madt + offset);
        if (record->entry_type == ENTRY_IO_APIC) {
            struct io_apic *io_apic = (struct io_apic *)record;
            mmio_dev_t dev_ioapic = {
                .addr = io_apic->io_apic_address,
                .size_bytes = 4096
            };
            add_mmio_device(dev_ioapic);
        }
        offset += record->record_length;
    }
}

static bool init_madt_devices() {
    madt = find_sdt("APIC");
    ASSERT(madt && verify_checksum((const uint8_t *)madt, madt->sdt.length), 0,
           INVALID_SDT("MADT"));

    lapic_addr = find_lapic_addr(madt);
    if (!lapic_addr) {
        return false;
    }

    init_apic_info(madt);
    init_devs_ioapic();

    mmio_dev_t dev_hpet = {
        .addr = lapic_addr,
        .size_bytes = 4096
    };

    add_mmio_device(dev_hpet);

    return true;
}

void init_acpi() {
    dev_info.num_devs = 0;
    struct limine_rsdp_response *rsdp_response = kernel_rsdp_request.response;

    rsdp = rsdp_response->address;
    rsdt = (struct RSDT *)((uint64_t)rsdp->rsdt_address);
    xsdt = (struct XSDT *)rsdp->xsdt_address;

    ASSERT(verify_checksum((const uint8_t *)rsdp, get_rsdp_size(rsdp)), 0,
           INVALID_SDT("RSDP"));

    if (is_xsdt(rsdp)) {
        ASSERT(verify_checksum((const uint8_t *)xsdt, xsdt->sdt.length), 0,
               INVALID_SDT("XSDT"));
    } else {
        ASSERT(verify_checksum((const uint8_t *)rsdt, rsdt->sdt.length), 0,
               INVALID_SDT("RSDT"));
    }

    ASSERT(init_madt_devices(), 0, "MADT Device Initialization Failure\n");

    if (!init_dev_hpet()) {
        is_hpet_present = false;
        ASSERT(false, 0, "HPET not present\n"); // Todo: use PIT if no HPET
    }

    kprintf(INFO GREEN"ACPI: Initialized\n");
}
