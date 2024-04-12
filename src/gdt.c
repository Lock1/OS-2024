#include <stdint.h>
#include <stdbool.h>
#include "header/cpu/gdt.h"
#include "header/cpu/interrupt.h"


static struct GlobalDescriptorTable global_descriptor_table = {
    .table = {
        {0},
        {
            .segment_high      = 0xF,
            .segment_low       = 0xFFFF,
            .base_high         = 0,
            .base_mid          = 0,
            .base_low          = 0,
            .access.non_system = 1,
            .access.type_bit   = 0xA,
            .access.privilege  = 0,
            .access.valid_bit  = 1,
            .opr_32_bit        = 1,
            .long_mode         = 0,
            .granularity       = 1,
        },
        {
            .segment_high      = 0xF,
            .segment_low       = 0xFFFF,
            .base_high         = 0,
            .base_mid          = 0,
            .base_low          = 0,
            .access.non_system = 1,
            .access.type_bit   = 0x2,
            .access.privilege  = 0,
            .access.valid_bit  = 1,
            .opr_32_bit        = 1,
            .long_mode         = 0,
            .granularity       = 1,
        },
        {
            .segment_high      = 0xF,
            .segment_low       = 0xFFFF,
            .base_high         = 0,
            .base_mid          = 0,
            .base_low          = 0,
            .access.non_system = 1,
            .access.type_bit   = 0xA,
            .access.privilege  = 3,
            .access.valid_bit  = 1,
            .opr_32_bit        = 1,
            .long_mode         = 0,
            .granularity       = 1,
        },
        {
            .segment_high      = 0xF,
            .segment_low       = 0xFFFF,
            .base_high         = 0,
            .base_mid          = 0,
            .base_low          = 0,
            .access.non_system = 1,
            .access.type_bit   = 0x2,
            .access.privilege  = 3,
            .access.valid_bit  = 1,
            .opr_32_bit        = 1,
            .long_mode         = 0,
            .granularity       = 1,
        },
        {
            .segment_high      = (sizeof(struct TSSEntry) & (0xF << 16)) >> 16,
            .segment_low       = sizeof(struct TSSEntry),
            .base_high         = 0,
            .base_mid          = 0,
            .base_low          = 0,
            .access.non_system = 0,    // S bit
            .access.type_bit   = 0x9,
            .access.privilege  = 0,    // DPL
            .access.valid_bit  = 1,    // P bit
            .opr_32_bit        = 1,    // D/B bit
            .long_mode         = 0,    // L bit
            .granularity       = 0,    // G bit
        },
        {0}
    }
};

struct GDTR _gdt_gdtr = {
    .address = &global_descriptor_table,
    .size    = sizeof(global_descriptor_table) - 1,
};

void gdt_install_tss(void) {
    uint32_t base = (uint32_t) &_interrupt_tss_entry;
    global_descriptor_table.table[5].base_high = (base & (0xFF << 24)) >> 24;
    global_descriptor_table.table[5].base_mid  = (base & (0xFF << 16)) >> 16;
    global_descriptor_table.table[5].base_low  = base & 0xFFFF;
}