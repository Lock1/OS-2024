#include <stdint.h>
#include <stdbool.h>
#include "header/cpu/gdt.h"
#include "header/cpu/idt.h"
#include "header/cpu/interrupt.h"
#include "header/kernel-entrypoint.h"
#include "header/text/framebuffer.h"

void kernel_setup(void) {
    load_gdt(&_gdt_gdtr);
    pic_remap();
    initialize_idt();
    framebuffer_clear();
    framebuffer_write(0, 0, 'a', 0x7, 0);
    framebuffer_write(0, 1, 'b', 0x7, 0);
    framebuffer_write(0, 2, 'c', 0x7, 0);
    framebuffer_set_cursor(0, 3);
    while (true);
}

// TODO: Quick Keyboard
// TODO: FAT32
// TODO: Paging
// TODO: U/K space
// TODO: Shell

// TODO: Context