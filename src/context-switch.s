global process_context_switch

; Load struct Context (CPU GP-register + CR3) then jump
; Function Signature: void process_context_switch(struct Context ctx);
process_context_switch:
    ; Using iret (return instruction for interrupt) technique for privilege change
    ; Stack values will be loaded into these register:
    ; [esp] -> eip, [esp+4] -> cs, [esp+8] -> eflags, [...] -> user esp, [...] -> user ss
    lea  ecx, [esp+0x04] ; Save the base address for struct Context ctx

    mov  eax, 0x20 | 0x3 ; GDT_USER_DATA_SELECTOR with user privilege
    push eax ; Stack segment selector (ss)

    mov  eax, [ecx+0x0C]
    push eax ; Stack pointer (esp)

    mov  eax, [ecx+0x34]
    push eax ; CPU flag register (eflags)

    mov  eax, 0x18 | 0x3 ; GDT_USER_CODE_SELECTOR with user privilege
    push eax ; Code segment selector (cs)

    mov  eax, [ecx+0x30]
    push eax ; Instruction counter register (eip)



    ; Setup all remaining general purpose (GP) register
    mov  edi, [ecx+0x00]
    mov  esi, [ecx+0x04]

    ; For esp, check iret stack
    mov  ebp, [ecx+0x08]

    mov  ebx, [ecx+0x10]
    mov  edx, [ecx+0x14]
    ; Deferred: eax & ecx

    mov  eax, [ecx+0x20]
    mov  gs, ax
    mov  eax, [ecx+0x24]
    mov  fs, ax
    mov  eax, [ecx+0x28]
    mov  es, ax
    mov  eax, [ecx+0x2C]
    mov  ds, ax

    ; We're done with the calculation, setup the eax & ecx
    mov  eax, [ecx+0x1C]
    mov  ecx, [ecx+0x18]

    iret
