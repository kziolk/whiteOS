[BITS 32]

; text section for this kernel code, so that it gets linked first


global _start
extern kernel_main

CODE_SEG equ 0x08
DATA_SEG equ 0x10

; this will be loaded into (void*) 0x00100000
_start:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebp, 0x00200000
    mov esp, ebp ; can overflow and reach kernel code


    ; Enable A20 line with Fast A20 gate
    in al, 0x92
    or al, 2
    out 0x92, al

    ; Remap the master pic. May not work on real hardware
    mov al, 00010001b
    out 0x20, al

    mov al, 0x20 ; this is where master isr should start
    out 0x21, al

    mov al, 00000001b
    out 0x21, al
    ; Ended remap

; calling main function from kernel.c
    call kernel_main

    jmp $

; for alignment
times 512-($ - $$) db 0
