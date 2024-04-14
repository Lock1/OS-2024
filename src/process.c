#include "header/process/process.h"
#include "header/memory/paging.h"
#include "header/stdlib/string.h"
#include "header/cpu/gdt.h"
#include "header/kernel-entrypoint.h"

static struct {
    struct ProcessControlBlock process_list[PROCESS_COUNT_MAX];
    uint32_t                   active_process_count;
    uint32_t                   available_pid;
} process_manager_state = {
    .process_list         = {[0 ... PROCESS_COUNT_MAX-1] = { .state = PROCESS_STATE_INACTIVE }},
    .active_process_count = 0,
    .available_pid        = 0,
};

static inline int32_t ceil_div(int32_t a, int32_t b) {
    return a / b + (a % b != 0);
}

static uint32_t process_generate_new_pid() {
    return process_manager_state.available_pid++;
}

// Assuming process_manager_state.active_process_count is properly updated
static int32_t process_list_get_inactive_index() {
    if (process_manager_state.active_process_count >= PROCESS_COUNT_MAX)
        return -1;
    for (int32_t i = 0; i < PROCESS_COUNT_MAX; ++i)
        if (process_manager_state.process_list[i].state == PROCESS_STATE_INACTIVE)
            return i;
    return -1;
}

// uint32_t process_destroy(uint32_t pid) {
//     process_manager_state.available_pid--;
// }


int32_t process_create_user_process(struct FAT32DriverRequest request) {
    __asm__ volatile("cli");
    int32_t retcode = 0; 
    if (process_manager_state.active_process_count >= PROCESS_COUNT_MAX) {
        retcode = 1;
        goto exit_cleanup;
    }

    // Check whether memory is enough for the executable and 1 frame each for user & kernel stack
    uint32_t page_frame_count_needed = ceil_div(request.buffer_size + 2*PAGE_FRAME_SIZE, PAGE_FRAME_SIZE);
    if (paging_allocate_check(page_frame_count_needed) || page_frame_count_needed > PROCESS_PAGE_FRAME_COUNT_MAX) {
        retcode = 2;
        goto exit_cleanup;
    }

    // Process metadata
    int32_t p_index = process_list_get_inactive_index();
    struct ProcessControlBlock *new_pcb = &(process_manager_state.process_list[p_index]);
    new_pcb->pid = process_generate_new_pid();
    memcpy(new_pcb->name, request.name, 8);
    new_pcb->state = PROCESS_STATE_ACTIVE;

    // Create new page directory for the process
    struct PageDirectory *new_page_dir    = paging_create_new_page_directory();
    new_pcb->context.page_directory_addr  = new_page_dir;
    new_pcb->memory.page_frame_used_count = page_frame_count_needed;
    for (uint32_t i = 0; i < page_frame_count_needed - 1; ++i) {
        void *virtual_addr = (void *) (i * PAGE_FRAME_SIZE);
        new_pcb->memory.virtual_addr_used[i] = virtual_addr;
        paging_allocate_user_page_frame(new_page_dir, virtual_addr);
    } 

    // Creating new kernel stack
    void *kernel_stack_virtual_addr = (void*) ((uint32_t) &_linker_kernel_virtual_base + KERNEL_RESERVED_PAGE_FRAME_COUNT*PAGE_FRAME_SIZE);
    new_pcb->memory.virtual_addr_used[page_frame_count_needed-1] = kernel_stack_virtual_addr;
    paging_allocate_kernel_page_frame(new_page_dir, kernel_stack_virtual_addr);

    // Read the executable to memory
    struct PageDirectory *current_page_dir = paging_get_current_page_directory_addr();
    paging_use_page_directory(new_page_dir);
    request.buf = (void*) 0;
    read(request);
    paging_use_page_directory(current_page_dir);

    // Context creation
    const uint32_t segment_data_register_value = GDT_USER_DATA_SEGMENT_SELECTOR | 0x3;
    struct Context new_context = {
        .cpu = {
            .general = {0},
            .index   = {0},
            .segment = {
                .ds = segment_data_register_value,
                .es = segment_data_register_value,
                .fs = segment_data_register_value,
                .gs = segment_data_register_value,
            },
            .stack = {
                // Top most of user call stack: (page_frame_count_needed-1)*PAGE_FRAME_SIZE
                // Get the top most address for call stack register based on system word width: - sizeof(int)
                .esp = (page_frame_count_needed-1)*PAGE_FRAME_SIZE - sizeof(int),
            }
        },
        .cs                  = GDT_USER_CODE_SEGMENT_SELECTOR | 0x3,
        .eip                 = 0,
        .page_directory_addr = new_page_dir,
        .kernel_esp          = kernel_stack_virtual_addr,
    };
    new_pcb->context = new_context;

    process_manager_state.active_process_count++;

exit_cleanup:
    __asm__ volatile("sti");
    return retcode;
}