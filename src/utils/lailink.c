#include <core/mm/heap.h>
#include <drivers/ACPI/RSDT.h>
#include <utils/error.h>
#include <vendor/printf.h>

// OS-specific functions.
void *laihost_malloc(size_t size) { return malloc(size); }
void *laihost_realloc(void *old, size_t newsize, size_t oldsize) {
    return realloc_plus(old, newsize, oldsize);
}
void laihost_free(void *tofree, UNUSED size_t size) { free(tofree); }

void laihost_log(int lvl, const char *msg) { dprintf("[%i] %s\n", lvl, msg); }

void laihost_panic(const char *msg) {
    trigger_psod(HN_ERR_LAI_PANIC, (char *)msg, NULL);
}

void *laihost_scan(char *sig, size_t index) {
    return find_nth_thingy(sig, index);
}