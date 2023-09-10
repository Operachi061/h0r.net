#include "acpi.h"
#include <drivers/Memory/scubadeeznutz.h>
#include <utils/logging/logger.h>
#include <utils/string.h>

struct XSDT *xsdt;
uint16_t xsdt_entries;
struct RSDP *rsdp;

struct SDTHeader *find_table(struct XSDT *xsdt, char *signature) {
    for (int t = 0; t < xsdt_entries; t++) {
        struct SDTHeader *newSDTHeader =
            (struct SDTHeader *)PHYS_TO_VIRT(xsdt->PointerToOtherSDT[t]);
        for (int i = 0; i < 4; i++) {
            if (newSDTHeader->Signature[i] != signature[i]) {
                break;
            }
            if (i == 3)
                return newSDTHeader;
        }
    }
    return 0;
}
void list_tables(struct XSDT *xsdt) {
    for (int t = 0; t < xsdt_entries; t++) {
        struct SDTHeader *newSDTHeader =
            (struct SDTHeader *)PHYS_TO_VIRT(xsdt->PointerToOtherSDT[t]);
        log_info("ACPI table #%u: %c%c%c%c", t, newSDTHeader->Signature[0],
                 newSDTHeader->Signature[1], newSDTHeader->Signature[2],
                 newSDTHeader->Signature[3]);
    }
}
bool do_checksum(struct XSDT *table) {
    // thanks to @schkwve on discord, he is the best
    uint8_t xsdp_checksum = 0;
    uint8_t *xsdp_ptr = (uint8_t *)table;

    for (uintptr_t i = 0; i < sizeof(struct XSDT); i++) {
        xsdp_checksum += xsdp_ptr[i];
    }

    return (xsdp_checksum & 0xFF) == 0;
}

void init_acpi(void *rsdp_addr) {
    rsdp = (struct RSDP *)rsdp_addr;
    if (!(rsdp->Signature[0] == 'R' && rsdp->Signature[1] == 'S' &&
          rsdp->Signature[2] == 'D' && rsdp->Signature[3] == ' ' &&
          rsdp->Signature[4] == 'P' && rsdp->Signature[5] == 'T' &&
          rsdp->Signature[6] == 'R')) {
        log_CRITICAL(NULL, HN_ERR_ACPI_FAULT, "RSDP not found or corrupted");
        return;
    }
    log_info("RSDP found");

    bool use_xsdt = false;
    if (rsdp->Revision != 0) {
        xsdt = (struct XSDT *)PHYS_TO_VIRT(rsdp->XSDTAddress);
        if (do_checksum(xsdt)) {
            log_info("XSDT found");
            xsdt_entries = (xsdt->h.Length - sizeof(xsdt)) / 8;
            use_xsdt = true;
        } else {
            log_error("XSDT corrupted ,falling back to RSDT");
        }
    } else {
        xsdt = (struct XSDT *)PHYS_TO_VIRT(rsdp->RSDTAddress);
        if (do_checksum(xsdt)) {
            log_info("RSDT found");
            xsdt_entries = (xsdt->h.Length - sizeof(xsdt)) / 4;
            use_xsdt = false;
        } else {
            log_CRITICAL(NULL, HN_ERR_ACPI_FAULT,
                         "RSDT corrupted, can not fall back to anything");
        }
    }

    if (use_xsdt) {
        list_tables(xsdt);

        struct SDTHeader *madt = find_table(xsdt, "APIC");
        log_info("MADT found");
        log_info("%u", madt->Length);

        log_info("XSDT CreatorID: %u", xsdt->h.CreatorID);
        log_info("XSDT CreatorRevision: %u", xsdt->h.CreatorRevision);
        log_info("XSDT Length: %u", xsdt->h.Length);
        log_info("XSDT Revision: %u", xsdt->h.Revision);
        log_info("XSDT Entries: %u", xsdt_entries);
    } else {
        log_CRITICAL(NULL, HN_ERR_UNIMPLEMENTED,
                     "Unimplemented RSDT parser, this is placeholder");
    }

    log_info("ACPI initialized successfully");
}
