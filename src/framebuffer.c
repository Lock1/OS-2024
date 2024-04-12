#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/text/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/cpu/portio.h"

uint16_t framebuffer_get_cursor(void) {
    uint16_t position = 0;
    out(CURSOR_PORT_CMD, CURSOR_PORT_CMD_LOWER_BYTE);
    position |= in(CURSOR_PORT_DATA);
    out(CURSOR_PORT_CMD, CURSOR_PORT_CMD_UPPER_BYTE);
    position |= ((uint16_t) in(CURSOR_PORT_DATA)) << 8;
    return position;
}

void framebuffer_set_cursor(uint8_t r, uint8_t c) {
    uint16_t location = r * 0x50 + c;
    out(CURSOR_PORT_CMD, CURSOR_PORT_CMD_UPPER_BYTE);
    out(CURSOR_PORT_DATA, (location & 0xFF00u) >> 8);
    out(CURSOR_PORT_CMD, CURSOR_PORT_CMD_LOWER_BYTE);
    out(CURSOR_PORT_DATA, location & 0x00FFu);
}

void framebuffer_write(uint8_t row, uint8_t col, char c, uint8_t fg, uint8_t bg) {
    register uint8_t background = (bg & 0xF) << 4;
    register uint8_t foreground = (fg & 0xF);
    register struct TextFramebufferCell cell = {
        .character = c,
        .color     = {
            .fg = foreground,
            .bg = background,
        },
    };
    FRAMEBUFFER_MEMORY_OFFSET[row][col] = cell;
}

void framebuffer_clear(void) {
    for (int i = 0; i < 25; ++i) {
        for (int j = 0; j < 80; ++j) {
            register struct TextFramebufferCell cell = {
                .character = '\0',
                .color     = {
                    .fg = FRAMEBUFFER_CLEAR_COLOR_FG,
                    .bg = FRAMEBUFFER_CLEAR_COLOR_BG,
                },
            };
            FRAMEBUFFER_MEMORY_OFFSET[i][j] = cell;
        }
    }
}
