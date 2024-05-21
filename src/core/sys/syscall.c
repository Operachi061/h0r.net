#include "syscall.h"
#include <libk/macros.h>
#include <utils/log.h>
#include <core/sys/sched/sched.h>

u64 syscall(u64 number,UNUSED u64 arg1,UNUSED u64 arg2,UNUSED u64 arg3)
{
    switch (number) {
        case 0: //sys_exit
            //TODO: exit codes
            sched_kill(sched_current_pid);
            // the function above will not return
        default:
            log_error("Syscall %d not implemented", number);
            return 0;
    }
}
