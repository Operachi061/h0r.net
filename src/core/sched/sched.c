#include "sched.h"
#include <core/kernel.h>
#include <core/logging/logger.h>
#include <drivers/Memory/PFA.h>
#include <drivers/Memory/scubadeeznutz.h>
#include <types/queue.h>
#include <types/vector.h>

typedef struct {
    Registers status;
    uint8_t pl_and_flags;
    void *cr3;
} process_t;

extern uint32_t kernel_end; // dont take its value, its just a label

bool is_sched_active;
uint64_t current_PID;
void *krnlcr3;

queue(uint64_t, process_queue);
vector_static(process_t, processes);

static bool not_first_tick;
void schedule(Registers *regs) {
    if (!is_sched_active)
        return;
    sched_next_process(regs);
}

void sched_next_process(Registers *regs) {
    __asm__ volatile("mov %0, %%cr3\r\n" : : "a"(krnlcr3));
    if (not_first_tick)
        memcpy(&processes.data[current_PID].status, regs, sizeof(Registers));
    not_first_tick = true;

    while (true) {
        if (process_queue.fullness == 0) {
            log_CRITICAL(NULL, HN_ERR_UNIMPLEMENTED, "Init system died lmao");
        }
        uint64_t next;
        queue_remove(process_queue, next);
        process_t nextproc = vector_at(&processes, next);
        if (nextproc.status.rip != 0) {
            current_PID = next;
            memcpy(regs, &nextproc.status, sizeof(Registers));
            __asm__ volatile("mov %0, %%cr3\r\n" : : "a"(nextproc.cr3));
            queue_add(process_queue, next);
            return;
        }
    }
}

void sched_init() {
    not_first_tick = false;
    __asm__ volatile("movq %%cr3, %0\r\n" : "=r"(krnlcr3) :);
    queue_init(process_queue, uint64_t, 500);
    vector_init(&processes);
}
void sched_enable() { is_sched_active = true; }

void kill_process(uint64_t ID) { processes.data[ID].status.rip = 0; }

uint64_t create_process(void (*process_main)(),
                        __attribute__((unused)) uint64_t prog_size,
                        uint8_t pl_and_flags, bool krnl_mode) {
    process_t newproc;
    newproc.pl_and_flags = pl_and_flags;
    if (krnl_mode) {
        __asm__ volatile("movq %%cr3, %0\r\n" : "=r"(newproc.cr3) :);
        newproc.status.rflags = 0x202;
        newproc.status.rip = (uint64_t)process_main;
        newproc.status.rsp = (uint64_t)request_pages(16);
        newproc.status.ss = 0x10;
        newproc.status.cs = 0x08;

    } else {
        // void *newcr3;
        // newcr3 = create_pml4();
        newproc.status.rsp = (uint64_t)request_pages(16);
        // scuba_map(newcr3, newproc.status.rsp,
        // VIRT_TO_PHYS(newproc.status.rsp),
        //           16, VIRT_FLAGS_USERMODE);
        newproc.status.rip = (uint64_t)process_main;
        // scuba_map(newcr3, (uint64_t)process_main, VIRT_TO_PHYS(process_main),
        //           NUM_BLOCKS(prog_size), VIRT_FLAGS_USERMODE);
        // scuba_map(newcr3, 0xffffffff80000000,
        // VIRT_TO_PHYS(0xffffffff80000000),
        //           NUM_BLOCKS((uint64_t)&kernel_end - 0xffffffff80000000),
        //           VIRT_FLAGS_USERMODE);
        // newproc.cr3 = (void *)((uint64_t)newcr3 - (uint64_t)data.hhdm_addr);
        __asm__ volatile("movq %%cr3, %0\r\n" : "=r"(newproc.cr3) :);
        newproc.status.rflags = 0x202;
        if ((pl_and_flags & 0x3) == 1) {
            newproc.status.ss = 0x18;
            newproc.status.cs = 0x20;
        } else if ((pl_and_flags & 0x3) == 2) {
            newproc.status.ss = 0x28;
            newproc.status.cs = 0x30;
        } else if ((pl_and_flags & 0x3) == 3) {
            newproc.status.ss = 0x38;
            newproc.status.cs = 0x40;
        }
    }

    vector_push_back(&processes, newproc);
    queue_add(process_queue, processes.len - 1);

    return processes.len - 1;
}