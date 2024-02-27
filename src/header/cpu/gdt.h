#ifndef _GDT_H
#define _GDT_H

#include <stdint.h>

// Some GDT Constant
#define GDT_MAX_ENTRY_COUNT 32

#define GDT_KERNEL_CODE_SEGMENT_SELECTOR 0x8
#define GDT_KERNEL_DATA_SEGMENT_SELECTOR 0x10

extern struct GDTR _gdt_gdtr;

struct SegmentDescriptorAccessByte {
    uint8_t type_bit   : 4;
    uint8_t non_system : 1;
    uint8_t privilege  : 2;
    uint8_t valid_bit  : 1;
} __attribute__((packed));

struct SegmentDescriptor {
    // First 32-bit
    uint16_t segment_low;
    uint16_t base_low;

    // Next 16-bit (Bit 32 to 47)
    uint8_t                            base_mid;
    struct SegmentDescriptorAccessByte access;

    // Next 8-bit (Bit 48 to 55)
    uint8_t segment_high : 4;
    uint8_t _reserved    : 1;
    uint8_t long_mode    : 1;
    uint8_t opr_32_bit   : 1;
    uint8_t granularity  : 1;

    // Last 8-bit (Bit 56 to 63)
    uint8_t base_high;
} __attribute__((packed));

struct GlobalDescriptorTable {
    struct SegmentDescriptor table[GDT_MAX_ENTRY_COUNT];
} __attribute__((packed));

struct GDTR {
    uint16_t                     size;
    struct GlobalDescriptorTable *address;
} __attribute__((packed));

#endif
