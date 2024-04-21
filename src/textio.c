#include <stdbool.h>

#include "header/text/framebuffer.h"
#include "header/text/textio.h"

static inline bool printable_char(char c) {
    return 0x1F < c && c < 0x7F;
}

void putchar(char c, uint8_t color) {
    bool is_printable = printable_char(c);
    if (is_printable || c == '\n') {
        uint16_t cursor_location = framebuffer_get_cursor();
        uint8_t row = cursor_location / 80;
        uint8_t col = cursor_location % 80;
        if (is_printable) {
            framebuffer_write(row, col++, c, color, 0);
        } else {
            row++;
            col = 0;
        }
        framebuffer_set_cursor(row, col);
    }
}

void puts(char *buf, int32_t count, uint8_t color) {
    uint16_t cursor_location = framebuffer_get_cursor();
    uint8_t row = cursor_location / 80;
    uint8_t col = cursor_location % 80;
    int i = 0;
    while (i < count && buf[i] != '\0') {
        if (buf[i] == '\n') {
            row++;
            col = 0;
        } else if (printable_char(buf[i])) {
            framebuffer_write(row, col++, buf[i], color, 0); // Only printables
        }
        i++;
    }
    framebuffer_set_cursor(row, col);
}

void puts_position(char *buf, uint8_t color, uint16_t location) {
    uint8_t row = location / 80;
    uint8_t col = location % 80;
    int i = 0;
    while (buf[i] != '\0') {
        if (printable_char(buf[i]))
            framebuffer_write(row, col++, buf[i], color, 0); // Only printables
        i++;
    }
}