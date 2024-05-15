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

size_t strlen(const char *ptr) {
    uint32_t i = 0;
    while (ptr[i] != '\0')
        i++;
    return i;
}

void puts(char *buf, uint8_t color) {
    syscall(6, (uint32_t) buf, strlen(buf), color);
}

#define BIOS_BLACK         0x0
#define BIOS_BLUE          0x1
#define BIOS_GREEN         0x2
#define BIOS_CYAN          0x3
#define BIOS_RED           0x4
#define BIOS_MAGENTA       0x5
#define BIOS_BROWN         0x6
#define BIOS_GRAY          0x7

#define BIOS_DARK_GRAY     0x8
#define BIOS_LIGHT_BLUE    0x9
#define BIOS_LIGHT_GREEN   0xA
#define BIOS_LIGHT_CYAN    0xB
#define BIOS_LIGHT_RED     0xC
#define BIOS_LIGHT_MAGENTA 0xD
#define BIOS_YELLOW        0xE
#define BIOS_WHITE         0xF

void fgets(char *buf, uint32_t count) {
    size_t ctr = 0;
    while (ctr < count) {
        char c;
        syscall(4, (uint32_t) &c, 0, 0);
        if (c == '\n') {
            syscall(6, (uint32_t) "\n", 0xF, 0);
            ctr = count;
        } else if (c) {
            syscall(5, (uint32_t) &c, 0xF, 0);
            buf[ctr++] = c;
        }
        buf[ctr] = '\0';
    }
}

uint8_t strcmp(const char *a, const char *b) {
    if (strlen(a) != strlen(b))
        return 1;
    else {
        uint32_t i = 0;
        while (a[i] != '\0') {
            if (a[i] != b[i])
                return 1;
            i++;
        }
        return 0;
    }
}

int main(void) {
    syscall(7, 0, 0, 0);
    char buf[16];
    while (true) {
        puts("Brush@OS-IF2230", BIOS_LIGHT_GREEN);
        puts(":", BIOS_GRAY);
        puts("/", BIOS_LIGHT_BLUE);
        puts("$ ", BIOS_GRAY);
        fgets(buf, 16);
        if (!strcmp(buf, "ls")) {
            puts("\n", BIOS_BLACK);
        } else if (!strcmp(buf, "clock")) {
            init_clock();
        }
    }

    return 0;
}