#ifndef _CMOS_H
#define _CMOS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Manual addition, UTC+7 -> Bangkok, Hanoi, Jakarta
#define CMOS_TIMEZONE 7



/**
 * CMOS Time information
 * 
 * @param second  [0,59]
 * @param minute  [0,59]
 * @param hour    [0,23] for 24-hour. [1,12] for 12-hour, highest bit is PM marker
 * @param weekday [1,7] with Sunday = 1, Monday = 2, etc
 * @param day     [1,31] day of month
 * @param month   [1,12]
 * @param year    [0,99]
 * @param century [19-20], Note: Untested
 */
struct CMOSTimeRTC {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t weekday;
    uint8_t day;
    uint8_t month;
    uint8_t year;
    uint8_t century;
};



/* --- CMOS Update & Getter --- */

/**
 * Update CMOS driver with latest CMOS data
 * 
 * @note Blocking I/O operation
 */
void cmos_fetch_update();

/**
 * Get current CMOS driver data. 
 * As the name implies, it's not guaranteed to be the latest data
 * 
 * @return Latest driver CMOS data
 */
struct CMOSTimeRTC cmos_get_current_driver_data();

#endif
