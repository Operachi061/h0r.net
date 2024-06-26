#include "wakeup.h"
#include "arch/general/paging.h"
#include "core/sys/krnlexec.h"
#include <config.h>

#include <core/sys/krnlexec.h>
#include <core/sys/resman/SIV.h>
#include <core/sys/resman/tty.h>

#include <core/mm/heap.h>
#include <core/mm/pmm.h>

#include <vendor/printf.h>

#include <arch/x86_64/interrupts/interrupts.h>
#include <core/sys/wakeup/gaia.h>
#include <drivers/ACPI/MCFG.h>

#include <utils/error.h>
#include <utils/log.h>

#include <drivers/filesys/tar.h>

#include <lai/core.h>

extern int ACPI_revision;

void wakeup_init_hw() {
    log_info("h0r.net identifies as v%u.%u.%u \"%s\"\n", KERNEL_VER_MAJOR,
             KERNEL_VER_MINOR, KERNEL_VER_PATCH, KERNEL_VER_CODENAME);

    pmm_init();
    init_printf_locks();
    tty_register((tty_t){2, 0, 0, 0, 0});

    siv_init();
    tar_init();

    // lai_set_acpi_revision(ACPI_revision);
    // lai_create_namespace();

    log_nice("Hardware sucessfully initialized!\n");
}

void wakeup_do_mounts() {
    if (siv_num_drives == 0) {
        trigger_psod(HN_ERR_NO_FS, "No filesystem found", NULL);
    }

    log_nice("Filesystem sucessfully initialized!\n");
}
void wakeup_startup() {
    // start Gaia: the userspace portion of Wakeup
    execute("Gaia", gaia_main, false);

    log_nice("Userland sucessfully initialized!\n");

    // kickstart the sched
    // WARN: Arch specific code in core/ + TODO: smh move to arch/ or make
    // architecture neutral
    __asm__("int $32");
}