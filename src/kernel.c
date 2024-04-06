#include <stdint.h>
#include <stdbool.h>
#include "header/cpu/gdt.h"
#include "header/cpu/idt.h"
#include "header/cpu/interrupt.h"
#include "header/kernel-entrypoint.h"
#include "header/driver/keyboard.h"
#include "header/text/framebuffer.h"
#include "header/filesystem/fat32.h"

void kernel_setup(void) {
    load_gdt(&_gdt_gdtr);
    pic_remap();
    initialize_idt();
    activate_keyboard_interrupt();
    initialize_filesystem_fat32();
    framebuffer_clear();
    framebuffer_set_cursor(0, 0);

    int row = 0, col = 0;
    keyboard_state_activate();
    while (true) {
        char c;
        get_keyboard_buffer(&c);
        if (c) {
            framebuffer_write(row, col, c, 0xF, 0);
            if (col >= FRAMEBUFFER_WIDTH) {
                ++row;
                col = 0;
            } else {
                ++col;
            }
           framebuffer_set_cursor(row, col);
        }
    }

    while (true);
}

// TODO: Paging
// TODO: U/K space
// TODO: Shell

// TODO: Context