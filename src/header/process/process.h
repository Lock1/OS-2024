#ifndef _PROCESS_H
#define _PROCESS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "header/cpu/interrupt.h"
#include "header/filesystem/fat32.h"

#define PROCESS_NAME_LENGTH_MAX          32
#define PROCESS_PAGE_FRAME_COUNT_MAX     8
#define PROCESS_COUNT_MAX                16

#define KERNEL_RESERVED_PAGE_FRAME_COUNT 4
#define KERNEL_VIRTUAL_ADDRESS_BASE      0xC0000000

#define CPU_EFLAGS_BASE_FLAG               0x2
#define CPU_EFLAGS_FLAG_CARRY              0x1
#define CPU_EFLAGS_FLAG_PARITY             0x4
#define CPU_EFLAGS_FLAG_AUX_CARRY          0x10
#define CPU_EFLAGS_FLAG_ZERO               0x40
#define CPU_EFLAGS_FLAG_SIGN               0x80
#define CPU_EFLAGS_FLAG_TRAP               0x100
#define CPU_EFLAGS_FLAG_INTERRUPT_ENABLE   0x200
#define CPU_EFLAGS_FLAG_DIRECTION          0x400
#define CPU_EFLAGS_FLAG_OVERFLOW           0x800
#define CPU_EFLAGS_FLAG_IO_PRIVILEGE       0x3000
#define CPU_EFLAGS_FLAG_NESTED_TASK        0x4000
#define CPU_EFLAGS_FLAG_MODE               0x8000
#define CPU_EFLAGS_FLAG_RESUME             0x10000
#define CPU_EFLAGS_FLAG_VIRTUAL_8086       0x20000
#define CPU_EFLAGS_FLAG_ALIGNMENT_CHECK    0x40000
#define CPU_EFLAGS_FLAG_VINTERRUPT_FLAG    0x80000
#define CPU_EFLAGS_FLAG_VINTERRUPT_PENDING 0x100000
#define CPU_EFLAGS_FLAG_CPUID_INSTRUCTION  0x200000
#define CPU_EFLAGS_FLAG_AES_SCHEDULE_LOAD  0x40000000
#define CPU_EFLAGS_FLAG_ALTER_INSTRUCTION  0x80000000



typedef enum PROCESS_STATE {
    PROCESS_STOPPED  = 0,
    PROCESS_RUNNNING = 1,
    PROCESS_WAITING  = 2,
} PROCESS_STATE;

/**
 * Contain information needed to be able to get interrupted and resumed later
 * 
 * @param page_directory_addr CPU register CR3, containing pointer to active page directory
 */
struct Context {
    struct CPURegister   cpu;
    uint32_t             eip;
    uint32_t             eflags;
    struct PageDirectory *page_directory_virtual_addr;
};

struct ProcessControlBlock {
    struct {
        uint32_t      pid;
        char          name[PROCESS_NAME_LENGTH_MAX];
        PROCESS_STATE state;
    } metadata;

    struct Context context;
    struct {
        void     *virtual_addr_used[PROCESS_PAGE_FRAME_COUNT_MAX];
        uint32_t page_frame_used_count;
    } memory;
};

extern struct ProcessControlBlock _process_list[PROCESS_COUNT_MAX];

// Warning: This procedure assumes no reentrancy & any interrupt
int32_t process_create_user_process(struct FAT32DriverRequest request);

#endif