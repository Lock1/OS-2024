#include "header/process/scheduler.h"
#include "header/cpu/interrupt.h"
#include "header/memory/paging.h"

void scheduler_init(void) {
    // activate_timer_interrupt();
}

void scheduler_switch_to_next_process(void) {
    struct Context ctx_to_switch = _process_list[0].context;
    paging_use_page_directory(ctx_to_switch.page_directory_virtual_addr);
    process_context_switch(ctx_to_switch);
}