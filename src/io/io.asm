global outb
global insb

outb:
    [bits 32]
    mov dx, [esp + 4]
    mov al, [esp + 8]
    out dx, al
    ret

insb:
    [bits 32]
    mov dx, [esp + 4]
    xor eax, eax
    in al, dx
    ret
