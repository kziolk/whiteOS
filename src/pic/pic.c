#include "pic.h"

#include <stddef.h>
#include "io/io.h"

#define PIC_1_CTRL 0x20
#define PIC_2_CTRL 0xA0
#define PIC_1_DATA 0x21
#define PIC_2_DATA 0xA1

void kb_init() {
        /* Get current master PIC interrupt mask */
    //unsigned char curmask_master = inb(0x21);

    /* 0xFD is 11111101 - enables only IRQ1 (keyboard) on master pic
       by clearing bit 1. bit is clear for enabled and bit is set for disabled */
    outb(0x64, 0xAE);
}

void pic_remap()
{
    /* ICW1 - begin initialization */
    outb(PIC_1_CTRL, 0x11);
    iowait();
    outb(PIC_2_CTRL, 0x11);
    iowait();
    /* ICW2 - remap offset address of idt_table */
    /*
    * In x86 protected mode, we have to remap the PICs beyond 0x20 because
    * Intel have designated the first 32 interrupts as "reserved" for cpu exceptions
    */
    outb(PIC_1_DATA, 0x20);
    iowait();
    outb(PIC_2_DATA, 0x28);
    iowait();
    /* ICW3 - setup cascading */
    outb(PIC_1_DATA, 0x04);
    iowait();
    outb(PIC_2_DATA, 0x02);
    iowait();
    /* ICW4 - environment info */
    outb(PIC_1_DATA, 0x01);
    iowait();
    outb(PIC_2_DATA, 0x01);
    iowait();
    /* Initialization finished */

    /* un mask interrupts */
    outb(PIC_1_DATA, 0xff);
    iowait();
    outb(PIC_2_DATA, 0xff);
    iowait();
}

void pic_mask(int irq) {
    uint8_t port;
    if (irq < 8) {
        port = PIC_1_DATA;
    } else {
        irq -= 8;
        port = PIC_2_DATA;
    }
    uint8_t mask = insb(port);
    outb(port, mask | (1 << irq));
}

void pic_unmask(int irq) {
    uint8_t port;
    if (irq < 8) {
        port = PIC_1_DATA;
    } else {
        irq -= 8;
        port = PIC_2_DATA;
    }
    uint8_t mask = insb(port);
    outb(port, mask & ~(1 << irq));
}

void pic_disable() {
    outb(PIC_1_DATA, 0xff);
    iowait();
    outb(PIC_2_DATA, 0xff);
    iowait();
}

void pic_sendEndOfInterrupt(int irq) {
    if (irq >= 8)
        outb(PIC_2_CTRL, 0x20);
    outb(PIC_1_CTRL, 0x20);
}

uint16_t pic_readIrqReqReg() {
    outb(PIC_1_CTRL, 0x0A);
    outb(PIC_2_CTRL, 0x0A);
    return (insb(PIC_1_CTRL) | insb(PIC_2_CTRL) << 8);
}

uint16_t pic_readInServiceReg() {
    outb(PIC_1_CTRL, 0x0B);
    outb(PIC_2_CTRL, 0x0B);
    return (insb(PIC_1_CTRL) | insb(PIC_2_CTRL) << 8);
}
