#ifndef _FRAMEBUFFER_H
#define _FRAMEBUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Port I/O constants
#define CURSOR_PORT_CMD            0x03D4
#define CURSOR_PORT_DATA           0x03D5
#define CURSOR_PORT_CMD_UPPER_BYTE 14
#define CURSOR_PORT_CMD_LOWER_BYTE 15

#define FRAMEBUFFER_WIDTH  80
#define FRAMEBUFFER_HEIGHT 25

/**
 * Struct for text framebuffer mode 3h.
 * Sacrifice speed and bitwise operation 
 * for verbosity and step-by-step operation.
 */
struct TextFramebufferCell {
    uint8_t character;
    struct {
        uint8_t fg: 4;
        uint8_t bg: 4;
    } __attribute__((packed)) color;
};

// TODO: Even more fancy C constant + linkerscript symbol alignment
// Various framebuffer constant
// Fancy struct with crappy C array-pointer casting
#define FRAMEBUFFER_MEMORY_OFFSET  ((struct TextFramebufferCell(*)[80]) 0xC00B8000)
#define FRAMEBUFFER_CLEAR_COLOR_FG 0x07
#define FRAMEBUFFER_CLEAR_COLOR_BG 0x00

/**
 * Terminal framebuffer
 * Resolution: 80x25
 * Starting at FRAMEBUFFER_MEMORY_OFFSET,
 * - Even number memory: Character, 8-bit
 * - Odd number memory:  Character color lower 4-bit, Background color upper 4-bit
*/

/**
 * Set framebuffer character and color with corresponding parameter values.
 * More details: https://en.wikipedia.org/wiki/BIOS_color_attributes
 *
 * @param row Vertical location (index start 0)
 * @param col Horizontal location (index start 0)
 * @param c   Character
 * @param fg  Foreground / Character color
 * @param bg  Background color
 */
void framebuffer_write(uint8_t row, uint8_t col, char c, uint8_t fg, uint8_t bg);

/**
 * Get cursor location. Divide with 80 for row and use modulo for column
 * 
 * @return uint16_t, position = y*80 + x
*/
uint16_t framebuffer_get_cursor(void);

/**
 * Set cursor to specified location. Row and column starts from 0
 * 
 * @param r row
 * @param c column
*/
void framebuffer_set_cursor(uint8_t r, uint8_t c);

/**
 * Set all cell in framebuffer character to 0x00 (empty character)
 * and color to 0x07 (gray character & black background)
 * Extra note: It's allowed to use different color palette for this
 *
 */
void framebuffer_clear(void);

#endif