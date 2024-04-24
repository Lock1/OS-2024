#include <stdint.h>
#include "header/filesystem/fat32.h"

void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

void init_clock(void) {
    struct FAT32DriverRequest request = {
        .buf                   = (uint8_t*) 0,
        .name                  = "clock\0\0\0",
        .ext                   = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = CLUSTER_SIZE,
    };
    int retcode = 0;
    syscall(8, (uint32_t) &request, (uint32_t) &retcode, 0);
    if (retcode == 0)
        syscall(6, (uint32_t) "Clock running..\n", 17, 0xF);
}

int main(void) {
    init_clock();

    syscall(7, 0, 0, 0);
    char buf;
    while (true) {
        syscall(4, (uint32_t) &buf, 0, 0);
        if (buf) syscall(5, (uint32_t) &buf, 0xF, 0);
    }

    return 0;
}