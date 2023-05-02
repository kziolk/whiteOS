ORG 0x7c00
BITS 16

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

_start:
    jmp short start
    nop
 times 33 db 0

start:
    jmp 0:init_ptrs ; code segment will be changed

db 'this a hidden text inside my bootloader hue hue hue'

init_ptrs:
    cli ; clear interrupts for initializing pointers
    mov ax, 0x00
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti ; done, enable interrupts

; message printing procedure
print_msg:
    ; move cursor to 0,0
    mov ah, 02h
    mov bh, 00h
    mov dh, 00h
    mov dl, 00h
    int 0x10
    ; set si to message memory
    mov si, message
    mov bx, 0
.loop:
    lodsb
    cmp al, 0
    je .done
    call print_char
    jmp .loop
.done:
    jmp load_protected

print_char:
    mov ah, 0eh
    int 0x10
    ret
    ; end of printing

     ; leaving 16 bit mode
load_protected:
    cli
    ; loading gdt (index for last and first mem addr)
    lgdt[gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:load32

gdt_start:
gdt_null:
    dd 0x0
    dd 0x0

; offset=0x08
gdt_code:       ; cs should point to this
    dw 0xffff   ; segment limit 0-15 bits
    dw 0        ; base first 0-15 bits
    db 0        ; base 16-23
    db 0x9a     ; access byte
    db 11001111b    ; high 4 bit flags and low 4bit flags
    db 0        ; base 24-31 bits
; offset=0x10
gdt_data:       ; ds, ss, es, fs, gs
    dw 0xffff   ; segment limit 0-15 bits
    dw 0        ; base first 0-15 bits
    db 0        ; base 16-23
    db 0x92     ; access byte
    db 11001111b    ; high 4 bit flags and low 4bit flags
    db 0        ; base 24-31 bits

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

[BITS 32]
load32:
    mov eax, 1
    mov ecx, 100
    mov edi, 0x0100000 ; my kernel.asm code
    call ata_lba_read
    jmp CODE_SEG:0x0100000

ata_lba_read:
    mov ebx, eax ; backup lba
    ; send the highest 8 bits of the lba to hard disk controller
    shr eax, 24
    or eax, 0xE0 ; select the master drive
    mov dx, 0x1F6
    out dx, al
    ; finished sending the highest 8bits of the lba

    ; send the total sectors to read
    mov eax, ecx
    mov dx, 0x1F2
    out dx, al
    ; finished sending total sectors to read

    ; send more bits of the lba
    mov eax, ebx ; restore the backup lba
    mov dx, 0x1F3
    out dx, al
    ; finished sending more bits of the lba

    ; send more bits of the lba
    mov dx, 0x1F4
    mov eax, ebx ; restore the backup lba
    shr eax, 8
    out dx, al
    ; finished sending more bits of the lba

    ; send upper 16 bits of the lba
    mov dx, 0x1F5
    mov eax, ebx
    shr eax, 16
    out dx, al
    ; finished sending upper 16 bits of the lba

    mov dx, 0x1F7
    mov al, 0x20
    out dx, al

    ; Read all sectors into memory
.next_sector:
    push ecx

.try_again:
    mov dx, 0x1F7
    in al, dx
    test al, 8
    jz .try_again

; we need to read 256 words at a time
    mov ecx, 256
    mov dx, 0x1F0
    rep insw
    pop ecx
    loop .next_sector
    ; End 
    ret

message: db 'hello from boot loader', 0xa, 0xd, 0

times 510-($ - $$) db 0
dw 0xAA55 ; Last two bytes (little endian) form the magic number,
            ; so that BIOS knows we are a boot sector.

