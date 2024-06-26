#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <arch/x86_64/interrupts/interrupts.h>
#include <libk/stdint.h>
#include <libk/stdbool.h>

enum process_state {
    SCHED_STATE_NONEXISTENT = 0,
    SCHED_STATE_READY = 1,
    SCHED_STATE_BLOCKED = 2,
    SCHED_STATE_DEAD = 3
};

typedef struct {
    u8 state_flags;
    char* name;
    //WARN: Arch specific code in core/ + TODO: smh move to arch/ or make architecture neutral
    Registers regs;
    Registers saved_regs;
    u32 tty_id;
    u64 pagemap;
} process_t;

#endif // __PROCESS_H__