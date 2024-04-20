global process_context_switch

; Load struct Context (CPU GP-register + CR3) then jump
; Function Signature: void process_context_switch(struct Context ctx);
process_context_switch:
    ; Using iret (return instruction for interrupt) technique for privilege change
    ; Stack values will be loaded into these register:
    ; [esp] -> eip, [esp+4] -> cs, [esp+8] -> eflags, [...] -> user esp, [...] -> user ss
    lea  ecx, [esp+4] ; Save the base address for struct Context ctx

    mov  eax, 0x20 | 0x3 ; GDT_USER_DATA_SELECTOR with user privilege
    push eax ; Stack segment selector (ss)

    mov  eax, [ecx+8]
    push eax ; Stack pointer (esp)

    mov  eax, [ecx+52]
    push eax ; CPU flag register (eflags)

    mov  eax, 0x18 | 0x3 ; GDT_USER_CODE_SELECTOR with user privilege
    push eax ; Code segment selector (cs)

    mov  eax, [ecx+48]
    push eax ; Instruction counter register (eip)

    ; Setup all remaining general purpose (GP) register
    mov  edi, [ecx+0]
    mov  esi, [ecx+4]

    ; For esp, check iret stack
    mov  ebp, [ecx+12]

    mov  ebx, [ecx+16]
    mov  edx, [ecx+20]
    ; Deferred: eax & ecx

    mov  eax, [ecx+32]
    mov  gs, ax
    mov  eax, [ecx+36]
    mov  fs, ax
    mov  eax, [ecx+40]
    mov  es, ax
    mov  eax, [ecx+44]
    mov  ds, ax

    ; We're done with the calculation, setup the eax & ecx
    mov  eax, [ecx+28]
    mov  ecx, [ecx+24]

    iret
