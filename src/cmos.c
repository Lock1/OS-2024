#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "header/driver/cmos.h"
#include "header/cpu/portio.h"

#define CMOS_COMMAND_PIO 0x70
#define CMOS_DATA_PIO    0x71

/**
 * For additional information check OSDev
 * Reference: https://wiki.osdev.org/CMOS
 */ 

static struct CMOSTimeRTC cached_cmos_value = {0};

#define CMOS_STATUS_REGISTER_B 0x0B
#define CMOS_STATUS_REGISTER_B_24_HR_MODE         0x2
#define CMOS_STATUS_REGISTER_B_BINARY_ACCESS_MODE 0x4
static void cmos_initialize() {
    out(CMOS_COMMAND_PIO, CMOS_STATUS_REGISTER_B);
    // Get current value of register B then set the flags
    register uint8_t register_b_value = in(CMOS_DATA_PIO); 
    register_b_value |= CMOS_STATUS_REGISTER_B_24_HR_MODE;
    register_b_value |= CMOS_STATUS_REGISTER_B_BINARY_ACCESS_MODE;
    out(CMOS_DATA_PIO, register_b_value);
}

#define CMOS_STATUS_REGISTER_A 0x0A
static bool cmos_check_update() {
    out(CMOS_COMMAND_PIO, CMOS_STATUS_REGISTER_A);
    return in(CMOS_DATA_PIO) & 0x80;
}
 
static uint8_t cmos_get_register(uint8_t cmos_register) {
    out(CMOS_COMMAND_PIO, cmos_register);
    return in(CMOS_DATA_PIO);
}
 
#define CMOS_SECOND_REGISTER  0x00
#define CMOS_MINUTE_REGISTER  0x02
#define CMOS_HOUR_REGISTER    0x04
#define CMOS_WEEK_REGISTER    0x06
#define CMOS_DAY_REGISTER     0x07
#define CMOS_MONTH_REGISTER   0x08
#define CMOS_YEAR_REGISTER    0x09
#define CMOS_CENTURY_REGISTER 0x32
void cmos_fetch_update() {
    cmos_initialize();

    while (cmos_check_update());
    cached_cmos_value.second  = cmos_get_register(CMOS_SECOND_REGISTER);
    cached_cmos_value.minute  = cmos_get_register(CMOS_MINUTE_REGISTER);
    cached_cmos_value.hour    = (cmos_get_register(CMOS_HOUR_REGISTER) + CMOS_TIMEZONE) % 24;
    cached_cmos_value.weekday = cmos_get_register(CMOS_WEEK_REGISTER);
    cached_cmos_value.day     = cmos_get_register(CMOS_DAY_REGISTER);
    cached_cmos_value.month   = cmos_get_register(CMOS_MONTH_REGISTER);
    cached_cmos_value.year    = cmos_get_register(CMOS_YEAR_REGISTER);
    cached_cmos_value.century = cmos_get_register(CMOS_CENTURY_REGISTER);
}

struct CMOSTimeRTC cmos_get_current_driver_data() {
    return cached_cmos_value;
}
