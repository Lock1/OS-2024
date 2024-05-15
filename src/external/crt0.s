global _start
extern main

section .text
_start:
    call main
    jmp  _exit

_exit:
    mov eax, 10
    int 0x30