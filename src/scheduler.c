#include "header/process/process.h"
#include "header/process/scheduler.h"
#include "header/cpu/interrupt.h"
#include "header/memory/paging.h"

static struct {
    uint32_t process_idx;
} scheduler_state = {
    .process_idx = 0,
};

void scheduler_init(void) {
    activate_timer_interrupt();
}

void scheduler_save_context_to_current_running_pcb(struct Context ctx) {
    struct ProcessControlBlock *running_pcb = process_get_current_running_pcb_pointer();
    running_pcb->context = ctx;
}

__attribute__((noreturn)) void scheduler_switch_to_next_process(void) {
    struct ProcessControlBlock *prev_running_pcb = process_get_current_running_pcb_pointer();
    struct ProcessControlBlock *next_running_pcb = NULL;

    if (prev_running_pcb != NULL)
        prev_running_pcb->metadata.state = PROCESS_WAITING;

    while (!next_running_pcb) {
        struct ProcessControlBlock *iter_pcb = &_process_list[scheduler_state.process_idx];
        if (iter_pcb->metadata.state == PROCESS_WAITING)
            next_running_pcb = iter_pcb;
        // This will set scheduler_state.process_idx as next process when next_running_pcb found
        scheduler_state.process_idx = (scheduler_state.process_idx + 1) % PROCESS_COUNT_MAX;
    }

    next_running_pcb->metadata.state = PROCESS_RUNNING;
    struct Context ctx_to_switch = next_running_pcb->context;
    paging_use_page_directory(ctx_to_switch.page_directory_virtual_addr);
    pic_ack(IRQ_TIMER);
    process_context_switch(ctx_to_switch);
}