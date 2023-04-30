#include <stddef.h>
#include <stdint.h>

#include "idt/idt.h"
#include "io/io.h"

#define PIC_1_CTRL 0x20
#define PIC_2_CTRL 0xA0
#define PIC_1_DATA 0x21
#define PIC_2_DATA 0xA1

size_t strlen(const char* str) 
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}
 
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
 
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color = 15 | 6 << 4;
uint16_t* terminal_buffer;

static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
	return (uint16_t) uc | (uint16_t) color << 8;
}
 
void terminal_initialize(void) 
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_buffer = (uint16_t*) 0xB8000;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			terminal_buffer[y * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
		}
	}
}
 
void terminal_putchar(char c) 
{
    if (c == '\n') {
        terminal_column = 0;
        terminal_row = (terminal_row + 1) % VGA_HEIGHT;
        return;
    }
    
    terminal_buffer[terminal_row * VGA_WIDTH + terminal_column] = vga_entry(c, terminal_color);
    terminal_column = (terminal_column + 1) % VGA_WIDTH;
    if (!terminal_column) {
        terminal_row = (terminal_row + 1) % VGA_HEIGHT;
	}
}
 
void terminal_writestring(const char* data) 
{
	int size = strlen(data);
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

void zero_interrupt() {
    terminal_writestring("Division by zero interrupt\n");
}

static void kb_init() {
        /* Get current master PIC interrupt mask */
    //unsigned char curmask_master = inb(0x21);

    /* 0xFD is 11111101 - enables only IRQ1 (keyboard) on master pic
       by clearing bit 1. bit is clear for enabled and bit is set for disabled */
    outb(0x64, 0xAE);
}

static void pic_remap()
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

// irq handler
void irq_init()
{
    pic_remap();
}

extern void int20h();
void kernel_main(void* multiboot_info_ptr, uint32_t magic_number) 
{
    /* Initialize terminal interface */
	//terminal_initialize();
    terminal_initialize();
    terminal_writestring("WhiteOS\n");
    
    //disable_interrupts();
    //irq_init();
    /*// Osdev RTC setting the registers
    disable_interrupts();		// important that no interrupts happen (perform a CLI)
    outb(0x70, 0x8A);	// select Status Register A, and disable NMI (by setting the 0x80 bit)
    outb(0x71, 0x20);	// write to CMOS/RTC RAM
    enable_interrupts();		// (perform an STI) and reenable NMI if you wish
    // turning on irq 8
    disable_interrupts();			// disable interrupts
    outb(0x70, 0x8B);		// select register B, and disable NMI
    char prev=insb(0x71);	// read the current value of register B
    outb(0x70, 0x8B);		// set the index again (a read will reset the index to register D)
    outb(0x71, prev | 0x40);	// write the previous value ORed with 0x40. This turns on bit 6 of register B
    enable_interrupts();
    */
    
    idt_init();    
    //set_idt_handler(0x00, zero_interrupt);
    //set_idt_handler(0x21, keyboard_interrupt); 
    //kb_init();
    enable_interrupts();
    
    div_by_zero();
    while (1) {
        int exception = fetchException();
        if (exception) {
            char exceptionStr[] = { '0' + exception, '\n', 0 };
            terminal_writestring("exception ");
            terminal_writestring(exceptionStr);
        }
    }
    

    terminal_writestring("\nexitting kernel\n");
}
