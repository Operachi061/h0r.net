#include "pmm.h"
#include <config.h>
#include <utils/error.h>
#include <utils/log.h>
#include <vendor/printf.h>

size_t total_mem = 0;
size_t free_mem = 0;
size_t used_mem = 0;
size_t reserved_mem = 0;
size_t highest_block = 0;

struct Bitmap page_bmp;

#define BIT_TO_PAGE(bit) ((size_t)bit * PAGE_SIZE)
#define PAGE_TO_BIT(page) ((size_t)page / PAGE_SIZE)

extern size_t kernel_start;
extern size_t kernel_end;

bool lock_page(void *addr) {
    size_t index = (size_t)addr / PAGE_SIZE;
    if (bitmap_get(page_bmp, index)) {
        return true;
    }
    if (bitmap_set(page_bmp, index, true)) {
        free_mem -= PAGE_SIZE;
        used_mem += PAGE_SIZE;
        return true;
    } else {
        return false;
    }
}
bool free_page(void *addr) {
    size_t index = (size_t)addr / PAGE_SIZE;
    if (!bitmap_get(page_bmp, index)) {
        return true;
    }
    if (bitmap_set(page_bmp, index, false)) {
        free_mem += PAGE_SIZE;
        used_mem -= PAGE_SIZE;
        return true;
    } else {
        return false;
    }
}
bool reserve_page(void *addr) {
    size_t index = (size_t)addr / PAGE_SIZE;
    if (bitmap_get(page_bmp, index)) {
        return true;
    }
    if (bitmap_set(page_bmp, index, true)) {
        free_mem -= PAGE_SIZE;
        reserved_mem += PAGE_SIZE;
        return true;
    } else {
        return false;
    }
}
bool unreserve_page(void *addr) {
    size_t index = (size_t)addr / PAGE_SIZE;
    if (!bitmap_get(page_bmp, index)) {
        return true;
    }
    if (bitmap_set(page_bmp, index, false)) {
        free_mem += PAGE_SIZE;
        reserved_mem -= PAGE_SIZE;
    } else {
        return false;
    }
    return true;
}
bool lock_pages(void *addr, size_t num) {
    for (size_t i = 0; i < num; i++) {
        if (!lock_page((void *)(addr + i * PAGE_SIZE)))
            return false;
    }
    return true;
}
bool free_pages(void *addr, size_t num) {
    for (size_t i = 0; i < num; i++) {
        if (!free_page((void *)(addr + i * PAGE_SIZE)))
            return false;
    }
    return true;
}
bool reserve_pages(void *addr, size_t num) {
    for (size_t i = 0; i < num; i++) {
        if (!reserve_page((void *)(addr + i * PAGE_SIZE)))
            return false;
    }
    return true;
}
bool unreserve_pages(void *addr, size_t num) {
    for (size_t i = 0; i < num; i++) {
        if (!unreserve_page((void *)(addr + i * PAGE_SIZE)))
            return false;
    }
    return true;
}

size_t get_free_RAM() { return free_mem; }
size_t get_used_RAM() { return used_mem; }
size_t get_total_RAM() { return total_mem; }

static void *find_free_range(size_t npages) {
    for (size_t addr = 0; addr <= page_bmp.size; addr++) {
        log_trace("Trying 0x%p\n", addr);
        for (size_t page = 0; page <= npages; page++) {
            if (bitmap_get(page_bmp, addr + PAGE_TO_BIT(page)))
                break;

            if (page == npages)
                return (void *)BIT_TO_PAGE(addr);
        }
    }

    trigger_psod(HN_ERR_OUT_OF_MEM, "Out of Memory", NULL);
    return NULL;
}

void *request_pages(size_t num) {
    log_trace("Allocating %d pages\n", num);
    void *PP = (void *)find_free_range(num);
    if (!lock_pages(PP, num)) {
        return NULL;
    }
    return PP;
}

#if XTRA_DEBUG
static char *get_entry_type(uint64_t type) {
    switch (type) {
    case LIMINE_MEMMAP_USABLE:
        return "Usable";
    case LIMINE_MEMMAP_RESERVED:
        return "Reserved";
    case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
        return "ACPI Reclaimable";
    case LIMINE_MEMMAP_ACPI_NVS:
        return "ACPI NVS";
    case LIMINE_MEMMAP_BAD_MEMORY:
        return "Bad Memory";
    case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
        return "Bootloader Reclaimable";
    case LIMINE_MEMMAP_KERNEL_AND_MODULES:
        return "Kernel and Modules";
    case LIMINE_MEMMAP_FRAMEBUFFER:
        return "Framebuffer";
    default:
        return "???";
    }
}
#endif

void pmm_init() {
    size_t page_bmp_size = 0;
    void *largest_free_memseg = NULL;
    size_t largest_free_memseg_size = 0;
    log_trace("Memory map segments:\n");
    for (size_t i = 0; i < memmap_request.response->entry_count; i++) {
        struct limine_memmap_entry *desc = memmap_request.response->entries[i];
        log_trace("  -type:\"%s\",base:0x%p,length:%u\n",
                  get_entry_type(desc->type), desc->base, desc->length);
        if (desc->type == LIMINE_MEMMAP_USABLE &&
            desc->length > largest_free_memseg_size) {
            largest_free_memseg = (void *)(desc->base);
            largest_free_memseg_size = desc->length;
            total_mem += desc->length;
        }
        page_bmp_size += (desc->length / (PAGE_SIZE * 8)) + 1;
    }
    free_mem = total_mem;

    if (page_bmp_size > largest_free_memseg_size)
        trigger_psod(HN_ERR_OUT_OF_MEM,
                     "Page bitmap does not fit in largest free segment", NULL);

    page_bmp.size = page_bmp_size;
    page_bmp.buffer = largest_free_memseg;

    lock_pages(page_bmp.buffer, (page_bmp.size / PAGE_SIZE) + 1);
    reserve_page((void *)0);
    for (size_t i = 0; i < memmap_request.response->entry_count; i++) {
        struct limine_memmap_entry *desc = memmap_request.response->entries[i];
        if (desc->type != LIMINE_MEMMAP_USABLE) {
            reserve_pages((void *)desc->base, desc->length / PAGE_SIZE + 1);
        } else {
            unreserve_pages((void *)desc->base, desc->length / PAGE_SIZE + 1);
        }
    }

    log_trace("PMM initialized\n");
    log_trace("Total RAM: %u KB\n", total_mem / 1024);
    log_trace("Free RAM: %u KB\n", free_mem / 1024);
    log_trace("Used RAM: %u KB\n", used_mem / 1024);
    log_trace("Reserved RAM: %u KB\n", reserved_mem / 1024);
}