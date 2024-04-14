#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/memory/paging.h"

__attribute__((aligned(0x1000))) struct PageDirectory _paging_kernel_page_directory = {
    .table = {
        [0] = {
            .flag.present_bit       = 1,
            .flag.write_bit         = 1,
            .flag.use_pagesize_4_mb = 1,
            .lower_address          = 0,
        },
        [0x300] = {
            .flag.present_bit       = 1,
            .flag.write_bit         = 1,
            .flag.use_pagesize_4_mb = 1,
            .lower_address          = 0,
        },
    }
};

static struct PageManagerState page_manager_state = {
    .page_frame_map        = {
        [0]                            = true,
        [1 ... PAGE_FRAME_MAX_COUNT-1] = false
    },
    .free_page_frame_count = PAGE_FRAME_MAX_COUNT,
};

void update_page_directory_entry(
    struct PageDirectory *page_dir,
    void *physical_addr, 
    void *virtual_addr, 
    struct PageDirectoryEntryFlag flag
) {
    uint32_t page_index = ((uint32_t) virtual_addr >> 22) & 0x3FF;
    page_dir->table[page_index].flag          = flag;
    page_dir->table[page_index].lower_address = ((uint32_t) physical_addr >> 22) & 0x3FF;
    flush_single_tlb(virtual_addr);
}

void flush_single_tlb(void *virtual_addr) {
    asm volatile("invlpg (%0)" : /* <Empty> */ : "b"(virtual_addr): "memory");
}



/* --- Memory Management --- */
bool paging_allocate_check(uint32_t amount) {
    return page_manager_state.free_page_frame_count >= amount;
}


bool paging_allocate_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr) {
    if (!paging_allocate_check(1))
        return false;
    
    for (int i = 0; i < PAGE_FRAME_MAX_COUNT; ++i) {
        if (!page_manager_state.page_frame_map[i]) {
            page_manager_state.page_frame_map[i] = true;
            page_manager_state.free_page_frame_count--;
            struct PageDirectoryEntryFlag flag = {
                .present_bit       = true,
                .write_bit         = true,
                .user_bit          = true,
                .use_pagesize_4_mb = true,
            };
            update_page_directory_entry(
                page_dir,
                (void*) (i * PAGE_FRAME_SIZE),
                virtual_addr,
                flag
            );
            return true;
        }
    }

    return false;
}

bool paging_free_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr) {
    uint32_t                  page_index = (uint32_t) virtual_addr >> 22;
    struct PageDirectoryEntry entry      = page_dir->table[page_index];
    if (!entry.flag.present_bit)
        return false;
    page_manager_state.page_frame_map[entry.lower_address] = false;
    page_manager_state.free_page_frame_count++;

    struct PageDirectoryEntryFlag flag = {0};
    update_page_directory_entry(
        page_dir,
        (void*) 0,
        virtual_addr,
        flag
    );
    return true;
}

#include "header/process/process.h"
#include "header/kernel-entrypoint.h"
#define PAGING_PROCESS_DIRECTORY_TABLE_MAX_COUNT 32

__attribute__((aligned(0x1000))) static struct PageDirectory page_directory_list[PAGING_PROCESS_DIRECTORY_TABLE_MAX_COUNT];
static struct {
    bool page_directory_used[PAGING_PROCESS_DIRECTORY_TABLE_MAX_COUNT];
} page_directory_manager = {
    .page_directory_used = {false},
};

struct PageDirectory* paging_create_new_page_directory(void) {
    for (uint32_t i = 0; i < PAGING_PROCESS_DIRECTORY_TABLE_MAX_COUNT; ++i) {
        if (!page_directory_manager.page_directory_used[i]) {
            page_directory_manager.page_directory_used[i] = true;
            return &page_directory_list[i];
        }
    }
    return NULL;
}

struct PageDirectory* paging_get_current_page_directory_addr(void) {
    struct PageDirectory* current_page_directory_addr;
    __asm__ volatile("mov %%cr3, %%eax" : "=r"(current_page_directory_addr):);
    return current_page_directory_addr;
}

bool paging_allocate_kernel_page_frame(struct PageDirectory *page_dir, void *virtual_addr) {
    if (!paging_allocate_check(1))
        return false;
    
    for (int i = 0; i < PAGE_FRAME_MAX_COUNT; ++i) {
        if (!page_manager_state.page_frame_map[i]) {
            page_manager_state.page_frame_map[i] = true;
            page_manager_state.free_page_frame_count--;
            struct PageDirectoryEntryFlag flag = {
                .present_bit       = true,
                .write_bit         = true,
                .user_bit          = false,
                .use_pagesize_4_mb = true,
            };
            update_page_directory_entry(
                page_dir,
                (void*) (i * PAGE_FRAME_SIZE),
                virtual_addr,
                flag
            );
            return true;
        }
    }

    return false;
}

void paging_use_page_directory(struct PageDirectory *page_dir) {
    __asm__  volatile("mov %0, %%cr3" : /* <Empty> */ : "r"(page_dir): "memory");
    for (uint32_t i = 0; i < PROCESS_PAGE_FRAME_COUNT_MAX; ++i) {
        void *target_virtual_addr = (void*) (i*PAGE_FRAME_SIZE);
        flush_single_tlb(target_virtual_addr);
        flush_single_tlb((void*) ((uint32_t) target_virtual_addr + _linker_kernel_virtual_base));
    }
}
