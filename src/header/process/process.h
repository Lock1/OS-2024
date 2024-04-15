#ifndef _PROCESS_H
#define _PROCESS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "header/cpu/interrupt.h"
#include "header/filesystem/fat32.h"

#define PROCESS_NAME_LENGTH_MAX      32
#define PROCESS_PAGE_FRAME_COUNT_MAX 8
#define PROCESS_COUNT_MAX            16

#define PROCESS_STATE_INACTIVE 0
#define PROCESS_STATE_ACTIVE   1

#define KERNEL_RESERVED_PAGE_FRAME_COUNT 4
#define KERNEL_VIRTUAL_ADDRESS_BASE      0xC0000000

/**
 * Contain information needed to be able to get interrupted and resumed later
 * 
 * @param page_directory_addr CPU register CR3, containing pointer to active page directory
 */
struct Context {
    struct CPURegister   cpu;
    uint32_t             eip;
    uint32_t             eflags;
    struct PageDirectory *page_directory_addr;
};

struct ProcessControlBlock {
    struct {
        uint32_t pid;
        char     name[PROCESS_NAME_LENGTH_MAX];
        uint32_t state;
    } metadata;

    struct Context context;
    struct {
        void     *virtual_addr_used[PROCESS_PAGE_FRAME_COUNT_MAX];
        uint32_t page_frame_used_count;
    } memory;
};

int32_t process_create_user_process(struct FAT32DriverRequest request);

#endif