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

#endif