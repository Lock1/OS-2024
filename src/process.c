#include "header/process/process.h"
#include "header/memory/paging.h"
#include "header/stdlib/string.h"
#include "header/cpu/gdt.h"
#include "header/kernel-entrypoint.h"

struct ProcessControlBlock _process_list[PROCESS_COUNT_MAX] = {
    [0 ... PROCESS_COUNT_MAX-1] = { .metadata.state = PROCESS_STOPPED }
};

static struct {
    uint32_t active_process_count;
    uint32_t available_pid;
} process_manager_state = {
    .active_process_count = 0,
    .available_pid        = 0,
};

static inline int32_t ceil_div(int32_t a, int32_t b) {
    return a / b + (a % b != 0);
}

static uint32_t process_generate_new_pid() {
    return process_manager_state.available_pid++;
}

static int32_t process_list_get_inactive_index() {
    if (process_manager_state.active_process_count >= PROCESS_COUNT_MAX)
        return -1;
    for (uint32_t i = 0; i < PROCESS_COUNT_MAX; ++i)
        if (_process_list[i].metadata.state == PROCESS_STOPPED)
            return i;
    return -1;
}

struct ProcessControlBlock* process_get_current_running_pcb_pointer(void) {
    for (uint32_t i = 0; i < PROCESS_COUNT_MAX; ++i)
        if (_process_list[i].metadata.state == PROCESS_RUNNING)
            return &_process_list[i];
    return NULL;
}


int32_t process_create_user_process(struct FAT32DriverRequest request) {
    int32_t retcode = 0; 
    if (process_manager_state.active_process_count >= PROCESS_COUNT_MAX) {
        retcode = 1;
        goto exit_cleanup;
    }

    // Ensure entrypoint is not located at kernel's section at higher half
    if ((uint32_t) request.buf >= _linker_kernel_virtual_base) {
        retcode = 2;
        goto exit_cleanup;
    }

    // Check whether memory is enough for the executable and additional frame for user stack
    uint32_t page_frame_count_needed = ceil_div(request.buffer_size + PAGE_FRAME_SIZE, PAGE_FRAME_SIZE);
    if (!paging_allocate_check(page_frame_count_needed) || page_frame_count_needed > PROCESS_PAGE_FRAME_COUNT_MAX) {
        retcode = 3;
        goto exit_cleanup;
    }

    // Process metadata
    int32_t p_index = process_list_get_inactive_index();
    struct ProcessControlBlock *new_pcb = &(_process_list[p_index]);
    new_pcb->metadata.pid = process_generate_new_pid();
    memcpy(new_pcb->metadata.name, request.name, 8);
    new_pcb->metadata.state = PROCESS_WAITING;

    // Create new page directory for the process. Allocate frame needed for exec + stack
    struct PageDirectory *new_page_dir = paging_create_new_page_directory();
    void                 *top_user_esp = (void*) ((uint32_t) &_linker_kernel_virtual_base - sizeof(int));
    new_pcb->context.page_directory_virtual_addr  = new_page_dir;
    new_pcb->memory.page_frame_used_count = page_frame_count_needed;
    for (uint32_t i = 0; i < page_frame_count_needed - 1; ++i) {
        void *virtual_addr = (void *) ((uint32_t) request.buf + i * PAGE_FRAME_SIZE);
        new_pcb->memory.virtual_addr_used[i] = virtual_addr;
        paging_allocate_user_page_frame(new_page_dir, virtual_addr);
    } 
    paging_allocate_user_page_frame(new_page_dir, top_user_esp);
    new_pcb->memory.virtual_addr_used[page_frame_count_needed-1] = top_user_esp;

    // Read the executable to memory
    struct PageDirectory *current_page_dir = paging_get_current_page_directory_addr();
    paging_use_page_directory(new_page_dir);
    read(request);
    paging_use_page_directory(current_page_dir);

    // Context creation
    const uint32_t segment_data_register_value = GDT_USER_DATA_SEGMENT_SELECTOR | 0x3;
    struct Context initial_context = {
        .cpu = {
            .general = {
                .eax = 0,
                .ebx = 0,
                .ecx = 0,
                .edx = 0,
            },
            .index   = {
                .edi = 0,
                .esi = 0,
            },
            .stack = {
                .esp = (uint32_t) top_user_esp,
                .ebp = (uint32_t) top_user_esp,
            },
            .segment = {
                .ds = segment_data_register_value,
                .es = segment_data_register_value,
                .fs = segment_data_register_value,
                .gs = segment_data_register_value,
            }
        },
        .eip                         = (uint32_t) request.buf,
        .eflags                      = CPU_EFLAGS_BASE_FLAG | CPU_EFLAGS_FLAG_INTERRUPT_ENABLE,
        .page_directory_virtual_addr = new_page_dir,
    };
    new_pcb->context = initial_context;

    // Process creation success
    process_manager_state.active_process_count++;

exit_cleanup:
    return retcode;
}

bool process_destroy(uint32_t pid) {
    for (uint32_t i = 0; i < PROCESS_COUNT_MAX; ++i) {
        if (_process_list[i].metadata.pid == pid) {
            // TODO: Release paging & PCB
            // TODO: SIGTERM + syscall_exit()
            process_manager_state.available_pid--;
            process_manager_state.active_process_count--;
            return true;
        }
    }
    return false;
}