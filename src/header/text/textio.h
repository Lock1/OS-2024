#ifndef _TEXTIO_H
#define _TEXTIO_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Write single character into screen
 * 
 * @param c     Character to print
 * @param color Text foreground color
 */
void putchar(char c, uint8_t color);

/**
 * Simply write null-terminated string to framebuffer at current position
 * 
 * @param buf   Pointer to null-terminated string buffer
 * @param count How many character count to write
 * @param color Text foreground color
 */
void puts(char *buf, int32_t count, uint8_t color);

/**
 * Quick clone of puts() for writing at certain position. Will not move the cursor
 * 
 * @param buf      Pointer to null-terminated string buffer
 * @param color    Text foreground color
 * @param location Position to write. Row = loc/80. Col = loc%80.
 */
void puts_position(char *buf, uint8_t color, uint16_t location);

#endif