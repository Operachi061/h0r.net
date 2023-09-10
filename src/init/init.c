#include <kernel.h>

#include <drivers/DriverManager.h>
#include <drivers/Memory/Heap.h>
#include <drivers/Memory/Memory.h>
#include <drivers/Memory/PFA.h>
#include <drivers/Memory/scubadeeznutz.h>

#include "arch/x86_64/GDT/gdt.h"
#include <arch/x86_64/interrupts/interrupts.h>

#include "utils/error-handling/falut-handler.h"
#include "utils/screen.h"
#include <arch/x86_64/PIT.h>
#include <interface/desh.h>
#include <sched/sched.h>
#include <utils/logging/logger.h>
#include <vendor/printf.h>

// Forward decls for drivers not worth making .h-s for
void spisr_init();

void init_HW() {
    // Initialize screen and logger
    logger_set_output(LOGGER_OUTPUT_COM1);
    InitScreen(data.framebuffer);
    log_info("Kernel Init Target reached: IO\n");

    // Gather Data
    get_cpu_capabilities(&data.cpu_info);
    sys_init_fpu();

    log_info("CPU Vendor ID: %s", data.cpu_info.vendor);
    log_info("CPU Family: %d", data.cpu_info.family);
    log_info("CPU Model: %d", data.cpu_info.model);
    log_info("CPU Stepping: %d", data.cpu_info.stepping);
    log_info("CPU Extended Family: %d", data.cpu_info.ext_family);
    log_info("CPU Extended Model: %d", data.cpu_info.ext_model);
    log_info("CPU Capabilities (EDX): 0x%.8X", data.cpu_info.features[0]);
    log_info("CPU Capabilities (ECX): 0x%.8X", data.cpu_info.features[1]);
    log_info("Kernel Init Target reached: CPU\n");

    // Initialize GDT
    load_default_gdt();
    log_info("Kernel Init Target reached: GDT\n");

    // initialize interrupts
    initialize_interrupts();
    init_falut_handler();
    log_info("Kernel Init Target reached: Interrupts\n");

    // Init Memory stuff
    mem_init();
    PFA_init();
    InitHeap(0x20000);
    // still no workie
    // TODO: fix scuba
    // scuba_init(data.memmap_resp, data.kernel_addr_resp);
    log_info("Kernel Init Target reached: Memory\n");

    // Init x86 PC specific drivers
    pit_init();
    time_get(&data.time);

    log_info("Date: %02d/%02d/%d", data.time.month, data.time.day_of_month,
             data.time.year);
    log_info("Time: %02d:%02d:%02d", data.time.hours, data.time.minutes,
             data.time.seconds);

    log_info("Kernel Init Target reached: x86 PC drivers\n");

    // Init the HW drivers
    init_drivers();
    spisr_init();
    log_info("Kernel Init Target reached: Misc Drivers\n");

    enable_interrupts();
    log_info("Kernel Initialized Successfully\n");
}

void spam_As();
void spam_Bs();
void init_sys() {
    printf("WE MADE IT LESSGOOOOO\n");
    create_process(spam_As);
    create_process(spam_Bs);
    // DeshInit();

    while (true) {
        // DeshUpdate();
    }
}
void spam_As() {
    for (size_t i = 0; i < 30; i++) {
        printf("A");
    }
    while (true) {
    }
}
void spam_Bs() {
    for (size_t i = 0; i < 30; i++) {
        printf("B");
    }
    while (true) {
    }
}

void init_sched() {
    sched_init();
    create_process(init_sys);
    sched_enable();
}