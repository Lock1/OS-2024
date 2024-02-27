#include <stdint.h>
#include <stdbool.h>
#include "header/cpu/gdt.h"

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
        {0}
    }
};

struct GDTR _gdt_gdtr = {
    .address = &global_descriptor_table,
    .size    = sizeof(global_descriptor_table) - 1,
};
