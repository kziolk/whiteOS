[bits 32]

global outb
global insb

global outw
global insw

outb:
    mov dx, [esp + 4]
    mov al, [esp + 8]
    out dx, al
    ret

insb:
    mov dx, [esp + 4]
    xor eax, eax
    in al, dx
    ret


outw:
    mov dx, [esp + 4]
    mov ax, [esp + 8]
    out dx, ax
    ret

insw:
    mov edx, [ebp+8]
    xor eax, eax
    in ax, dx
    ret
