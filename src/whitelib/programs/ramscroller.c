#include "ramscroller.h"
#include "idt/idt.h"
#include "whitelib/string.h"
#include "terminal/terminal.h"
#include "keyboard/keyboard.h"
#include "memory/heap/kheap.h"

#include <stdint.h>

static void print_clock();
static void scan_keys();
static void print_ram();

static int* clock_ptr;
static int old_clock_val;

static uint8_t keyUp, keyUp2, keyUp3, keyDown, keyDown2, keyDown3, keyEsc;
static uint8_t keySpace, keyDot, keyEnter;
static char numstr_buff[32];
static intptr_t mem_addr;

static void init()
{
    keyUp = 17;
    keyUp2 = 18;
    keyUp3 = 19;
    keyDown = 31;
    keyDown2 = 32;
    keyDown3 = 33;
    keyEsc = 1;
    keySpace = 57;
    keyEnter = 28;
    keyDot = 52;
    clock_ptr = idt_get_tick_counter();
    old_clock_val = -1;
    mem_addr = 0;
}

static void getinput_number(char* str)
{
    uint8_t key;
    int keep_running = 1;
    while (keep_running) {
        key = keyboard_scan_key();
        if (key == keyEnter) keep_running = 0;
        else if (key >=2 && key <= 11) {
            *str = '0' + (key - 1) % 10;
            terminal_writechar(*str);
            str++;
        }
    }
    *str = '\0';
    terminal_print("\nok\n");
}

void ramscroller()
{
    init();
    print_ram();

    int keep_running = 1;
    while(keep_running) {
        // wait for the key code from key interrupt
        uint8_t key = keyboard_scan_key();
        // move up or down
        if (key == keyUp && mem_addr >= 4 * 4) mem_addr -= 4 * 4;
        else if (key == keyUp2 && mem_addr >= 4 * 4 * 16) mem_addr -= 4 * 4 * 16;
        else if (key == keyUp3 && mem_addr >= 4 * 4 * 256) mem_addr -= 4 * 4 * 256;
        else if (key == keyDown) mem_addr += 4 * 4;
        else if (key == keyDown2) mem_addr += 4 * 4 * 16;
        else if (key == keyDown3) mem_addr += 4 * 4 * 256;
        // toggle the while loop flag on escape
        else if (key == keyEsc) keep_running = 0;
        // '.' test the malloc
        else if (key == keyDot) {
            terminal_clear();
            terminal_print("How many bytes to allocate: ");
            getinput_number(numstr_buff);
            int val = toInt(numstr_buff);
            terminal_print("\nAllocating ");
            terminal_print(numstr_buff);
            terminal_print(" bytes\n");

            void* ptr = kmalloc(val);

            toStringHex((intptr_t)ptr, numstr_buff);
            terminal_print("Allocated memory on address: ");
            terminal_print(numstr_buff);

            terminal_print("\nPress key to continue\n");
            keyboard_scan_key(); // key down
            keyboard_scan_key(); // key up
        } else if (key == keyEnter) {
            terminal_clear();
            terminal_print("Go to (decimal) memory addr: ");
            getinput_number(numstr_buff);
            int val = toInt(numstr_buff);
            mem_addr = val;
        }
        print_ram();
    }
}

static void print_clock()
{
    toString(*clock_ptr, numstr_buff);
    terminal_gotoxy(VGA_WIDTH - 16, 0);
    terminal_print("Clock: ");
    terminal_print(numstr_buff);
}

static void scan_keys()
{
    terminal_print("press Key Up...\n");
    keyUp = keyboard_scan_key();
    keyboard_scan_key(); // for key release
    toString(keyUp, numstr_buff);
    terminal_print("scan number for Key Up: ");
    terminal_print(numstr_buff);
}

static void print_ram()
{
    terminal_clear();
    terminal_print("Keys: [wer]=up, [sdf]=down, .=test malloc, Enter=jump to addr, Escape = quit\n");
    terminal_print("RAM addr   | Hex Values:                                     | Char Values:\n");

    for (int row = 0; row < 20; row++) {
        terminal_gotoxy(0, 2 + row);
        toStringHex(mem_addr + 4 * row * 4, numstr_buff);
        terminal_print(numstr_buff);
        terminal_writechar('|');

        for (int col = 0; col < 4; col++) {
            uint32_t mem_val = ((uint32_t*)(mem_addr + 4 * col + 4 * row * 4))[0];
            toStringHex(mem_val, numstr_buff);

            terminal_gotoxy(13 + 12 * col, 2 + row);
            terminal_print(numstr_buff);

            terminal_gotoxy(63 + col * 4, 2 + row);
            terminal_putchar((mem_val >> 0) & 0xff);
            terminal_gotoxy(64 + col * 4, 2 + row);
            terminal_putchar((mem_val >> 8) & 0xff);
            terminal_gotoxy(65 + col * 4, 2 + row);
            terminal_putchar((mem_val >> 16) & 0xff);
            terminal_gotoxy(66 + col * 4, 2 + row);
            terminal_putchar((mem_val >> 24) & 0xff);
        }
    }
    terminal_print("\n\nvoid print_ram() addr: ");
    toStringHex((intptr_t)print_ram, numstr_buff);
    terminal_print(numstr_buff);
}
