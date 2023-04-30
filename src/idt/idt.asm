[bits 32]

extern dummy_handler
extern clock_handler
extern keyboard_handler

global load_idt
global disable_interrupts
global enable_interrupts

global idt_exception_handler
global idt_dummy_handler
global idt_clock_handler
global idt_keyboard_handler

global div_by_zero
div_by_zero:
    xor eax, eax
    div eax
    ret

load_idt:
    ; make new call frame
    push ebp             ; save old call frame
    mov ebp, esp         ; initialize new call frame

    ; load idt
    mov eax, [ebp + 8]
    lidt [eax]

    ; restore old call frame
    mov esp, ebp
    pop ebp
    ret

disable_interrupts:
    cli
    ret

enable_interrupts:
    sti
    ret

idt_dummy_handler:
    cli
    pushad
    call dummy_handler
    popad
    sti
    iret

idt_clock_handler:
    cli
    pushad
    call clock_handler
    popad
    sti
    iret

idt_keyboard_handler:
    push eax
    xor eax, eax
    in al, 60h
    
    push eax
    call keyboard_handler
    add esp, 4

    mov al, 20h
    out 20h, al
    pop eax
    iret

%macro isr_err_stub 1
isr_stub_%+%1:
    mov eax, %+%1
    push eax
    call exception_handler
    add esp, 4
    iret
%endmacro
; if writing for 64-bit, use iretq instead
%macro isr_no_err_stub 1
isr_stub_%+%1:
    mov eax, %+%1
    push eax
    call exception_handler
    add esp, 4
    iret
%endmacro

extern exception_handler
isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31

global isr_stub_table
isr_stub_table:
%assign i 0
%rep    32
    dd isr_stub_%+i ; use DQ instead if targeting 64-bit
%assign i i+1
%endrep
