#include "header/process/scheduler.h"
#include "header/cpu/interrupt.h"
#include "header/memory/paging.h"

void scheduler_init(void) {
    activate_timer_interrupt();
}

void scheduler_save_context_to_current_pcb(struct Context ctx) {
    _process_list[0].context = ctx;
    // TODO : Search
}

// TODO : Ensure process state is more or less UNIX like
void scheduler_switch_to_next_process(void) {
    // TODO : Scheduler & select waiting
    struct Context ctx_to_switch = _process_list[0].context;
    paging_use_page_directory(ctx_to_switch.page_directory_virtual_addr);
    pic_ack(IRQ_TIMER);
    process_context_switch(ctx_to_switch);
}