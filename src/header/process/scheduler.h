#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "header/process/process.h"

extern void process_context_switch(struct Context ctx);

// Includes preparing the timer interrupt
void scheduler_init(void); 
void scheduler_save_context_to_current_pcb(struct Context ctx);
void scheduler_switch_to_next_process(void);


#endif