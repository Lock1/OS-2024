#include <stdint.h>
#include <stdbool.h>

#include "header/driver/cmos.h"

void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

static inline void less_than_100_to_str(char *buf, int8_t x) {
    buf[0] = (x / 10) + '0';
    buf[1] = (x % 10) + '0';
}

int main(void) {
    struct CMOSTimeRTC time;
    char strbuf[32] = "XX:XX:XX";

    while (true) {
        syscall(12, (uint32_t) &time, 0, 0);
        less_than_100_to_str(strbuf+0, time.hour);
        less_than_100_to_str(strbuf+3, time.minute);
        less_than_100_to_str(strbuf+6, time.second);
        syscall(11, (uint32_t) strbuf, 0xF, 24*80 + 72);
    }

    return 0;
}