#include "idt.h"
#include "config.h"
#include "io/io.h"
#include "terminal/terminal.h"
#include "whitelib/system.h"

#include <stdint.h>


extern void load_idt(void*);
extern void* isr_stub_table[];

struct idt_entry
{
    unsigned short int offset_lowerbits;
    unsigned short int selector;
    unsigned char zero;
    unsigned char flags;
    unsigned short int offset_higherbits;
} __attribute__((packed));

struct idt_pointer
{
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

struct idt_entry idt_table[WHITEOS_TOTAL_INTERRUPTS];
struct idt_pointer idt_ptr;
void* idt_handlers[WHITEOS_TOTAL_INTERRUPTS];
int idt_tick_counter = 0;

static void init_idt_entry(int isr_number, short int selector, unsigned char flags)
{
    unsigned long base = (unsigned long) idt_handlers[isr_number];
    idt_table[isr_number].offset_lowerbits = base & 0xFFFF;
    idt_table[isr_number].offset_higherbits = (base >> 16) & 0xFFFF;
    idt_table[isr_number].selector = selector;
    idt_table[isr_number].flags = flags;
    idt_table[isr_number].zero = 0;
}

static void initialize_idt_pointer()
{
    idt_ptr.limit = (sizeof(struct idt_entry) * WHITEOS_TOTAL_INTERRUPTS) - 1;
    idt_ptr.base = (intptr_t)&idt_table;
}

// attach function and flags to given isr
void idt_set_handler(int isr_number, void* handler, uint8_t flags)
{
    idt_handlers[isr_number] = handler;
    init_idt_entry(isr_number, 0x8, flags);
}

void dummy_handler()
{
    terminal_print("dummy_idt ");
    outb(0x20, 0x20);
}

void clock_handler()
{
    idt_tick_counter++;
    outb(0x20, 0x20);
}

int* idt_get_tick_counter()
{
    return &idt_tick_counter;
}

void keyboard_handler(uint32_t key)
{
    system_push_event(SYS_EVENT_KEYBOARD, key);
    outb(0x20, 0x20);
}

static int exception_table[100];
static int exception_table_head = 0;
static int exception_table_tail = 0;
// exported for idt.asm to handle first 32 idt interrupts
void exception_handler(int etype)
{
    char str[] = { '0' + etype, ' ', 0};
    terminal_print("\nException: ");
    terminal_print(str);
    terminal_writechar('\n');

    exception_table[exception_table_tail] = etype + 1;
    exception_table_tail = (exception_table_tail + 1) % 100;
}

static void exception_table_init()
{
    for (int i = 0; i < 100; i++) {
        exception_table[i] = 0;
    }
}

void idt_init()
{
    terminal_print("idt_init()\n");
    initialize_idt_pointer();

    for (int i = 0; i < 32; i++) {
        idt_set_handler(i, isr_stub_table[i], 0x8e);
    }

    for (int i = 32; i < WHITEOS_TOTAL_INTERRUPTS; i++) {
        idt_set_handler(i, idt_dummy_handler, 0x8e);
    }
    idt_tick_counter = 0;
    idt_set_handler(0x20, idt_clock_handler, 0x8e);
    idt_set_handler(0x21, idt_keyboard_handler, 0x8e);

    load_idt(&idt_ptr);
   // __asm__ volatile ("lidt %0" : : "m"(idt_ptr)); // load the new IDT

    exception_table_init();
}


// for testing
int fetchException()
{
    int exception_number = exception_table[exception_table_head];

    if (exception_number) {
        exception_table[exception_table_head] = 0;
        exception_table_head = (exception_table_head + 1) % 100;
        char str[] = {
            'h', ':',
            '0' + exception_table_head / 10,
            '0' + exception_table_head%10,
            ' ', 0};
        terminal_print(str);
    }

    return exception_number;
}
