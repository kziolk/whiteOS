#include "idt.h"
#include "io/io.h"
#include "kernel.h"
#include <stdint.h>

#define IDT_SIZE 512 

extern void load_idt(void*);
extern void* isr_stub_table[];

struct idt_entry {
    unsigned short int offset_lowerbits;
    unsigned short int selector;
    unsigned char zero;
    unsigned char flags;
    unsigned short int offset_higherbits;
} __attribute__((packed));

struct idt_pointer {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

struct idt_entry idt_table[IDT_SIZE];
struct idt_pointer idt_ptr;
void* idt_handlers[IDT_SIZE];

static void init_idt_entry(int isr_number, short int selector, unsigned char flags) {
    unsigned long base = (unsigned long) idt_handlers[isr_number];
    idt_table[isr_number].offset_lowerbits = base & 0xFFFF;
    idt_table[isr_number].offset_higherbits = (base >> 16) & 0xFFFF;
    idt_table[isr_number].selector = selector;
    idt_table[isr_number].flags = flags;
    idt_table[isr_number].zero = 0;
}

static void initialize_idt_pointer() {
    idt_ptr.limit = (sizeof(struct idt_entry) * IDT_SIZE) - 1;
    idt_ptr.base = (unsigned int)&idt_table;
}

void idt_set_handler(int isr_number, void* handler, uint8_t flags) {
    idt_handlers[isr_number] = handler;
    init_idt_entry(isr_number, 0x8, flags);
}

void dummy_handler() {
    terminal_writestring("dummy interrupt ");
    outb(0x20, 0x20);
}

void clock_handler() {
    terminal_writestring(".");
    outb(0x20, 0x20);
}

void keyboard_handler(uint32_t key) {
    signed char keycode = key;
    char msg[] = { 'k', 'b', 'h' ,'i', 't' , ' ', '0' + (keycode % 10), '\n', 0 };
    terminal_writestring(msg);
    outb(0x20, 0x20);
}

int exception_table[100];
int exception_table_head = 0;
int exception_table_tail = 0;
void exception_handler(int etype) {
    char str[] = { '0' + etype, ' ', 0};
    terminal_writestring("etype = ");
    terminal_writestring(str);

    exception_table[exception_table_tail] = etype + 1;
    exception_table_tail = (exception_table_tail + 1) % 100;
}

void exception_table_init();
void idt_init() {
    terminal_writestring("idt_init()");
    initialize_idt_pointer();
    
    for (int i = 0; i < 32; i++) {
        idt_set_handler(i, isr_stub_table[i], 0x8e);
    }

    for (int i = 32; i < IDT_SIZE; i++) {
        idt_set_handler(i, idt_dummy_handler, 0x8e);
    }    
    idt_set_handler(0x20, idt_clock_handler, 0x8e);
    idt_set_handler(0x21, idt_keyboard_handler, 0x8e);

    load_idt(&idt_ptr);
   // __asm__ volatile ("lidt %0" : : "m"(idt_ptr)); // load the new IDT
    
    exception_table_init();
}

void exception_table_init() {
    for (int i = 0; i < 100; i++) {
        exception_table[i] = 0;
    }
}

int fetchException() {
    int exception_number = exception_table[exception_table_head];
    
    if (exception_number) {
        exception_table[exception_table_head] = 0;
        exception_table_head = (exception_table_head + 1) % 100;
        char str[] = {
            'h', ':', 
            '0' + exception_table_head / 10, 
            '0' + exception_table_head%10, 
            ' ', 0};
        terminal_writestring(str);
    }

    return exception_number;
}
