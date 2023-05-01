#include "defscreen.h"
#include "idt/idt.h"
#include "whitelib/string.h"
#include "terminal/terminal.h"
#include "keyboard/keyboard.h"
#include "memory/heap/kheap.h"

#include <stdint.h>

extern void stack_ptr();

static void print_clock();
static void print_test();
static void stack_test();
static void scan_keys();
static void print_ram();

static int* clock_ptr;
static int old_clock_val;

static uint32_t keyUp, keyUp2, keyUp3, keyDown, keyDown2, keyDown3, keyEsc;
static uint32_t keySpace;
static char numstr_buff[32];
static uint32_t mem_addr;

void defscreen_start() {
    ((char*)(2000))[0] = 'S';
    ((char*)(2000))[1] = 'i';
    ((char*)(2000))[2] = 'e';
    ((char*)(2000))[3] = 'm';
    ((char*)(2000))[4] = 'a';
    ((char*)(2000))[5] = '\n';

    clock_ptr = idt_get_tick_counter();
    old_clock_val = -1;
    mem_addr = 0;

    scan_keys();
    keyUp = 17;
    keyUp2 = 18;
    keyUp3 = 19;
    keyDown = 31;
    keyDown2 = 32;
    keyDown3 = 33;
    keyEsc = 1;
    keySpace = 57;

    print_ram(); 
    
    int keep_running = 1;
    while(keep_running) {
        uint32_t key = keyboard_scan_key();
        if (key == keyUp && mem_addr >= 4 * 4) mem_addr -= 4 * 4;
        else if (key == keyUp2 && mem_addr >= 4 * 4 * 16) mem_addr -= 4 * 4 * 16;
        else if (key == keyUp3 && mem_addr >= 4 * 4 * 256) mem_addr -= 4 * 4 * 256;
        else if (key == keyDown) mem_addr += 4 * 4;
        else if (key == keyDown2) mem_addr += 4 * 4 * 16;
        else if (key == keyDown3) mem_addr += 4 * 4 * 256;
        else if (key == keyEsc) keep_running = 0;
        else if (key == keySpace) {
            terminal_clear();
            div_by_zero();
            int a = 7;
            a -= key;
            a = 1 / a;
            break;
        } else if (key == 28) {
            terminal_clear();
            terminal_print("How many bytes to allocate: ");
            int val = 0;
            while (1) {
                key = keyboard_scan_key();
                if (key == 28) break;
                else if (key >= 2 && key <= 11) {
                    val *= 10;
                    val += (key-1) % 10;
                    terminal_writechar('0' + (key-1)%10);
                }
            }
            terminal_print("\nAllocating ");
            toString(val, numstr_buff);
            terminal_print(numstr_buff);
            terminal_print(" bytes\n");
            
            void* ptr = kmalloc(val);

            toStringHex((int)ptr, numstr_buff);
            terminal_print("Allocated memory on address: ");
            terminal_print(numstr_buff);

            terminal_print("\nPress key to continue\n");
            keyboard_scan_key();
            keyboard_scan_key();
        } else if (key == 52) {
            terminal_print("Go to memory addr: ");
            int val = 0;
            while (1) {
                key = keyboard_scan_key();
                if (key == 28) break;
                else if (key >= 2 && key <= 11) {
                    val *= 10;
                    val += (key-1) % 10;
                    terminal_writechar('0' + (key-1)%10);
                }
            }
            mem_addr = val;
        }
        print_ram();
    }
}

static void print_clock() {
    toString(*clock_ptr, numstr_buff);
    terminal_gotoxy(VGA_WIDTH - 16, 0);
    terminal_print("Clock: ");
    terminal_print(numstr_buff);
}

static void print_test() {
    terminal_gotoxy(0, 3);
    toStringHex(0xdead01, numstr_buff);
    terminal_print(numstr_buff);
    terminal_gotoxy(0, 4);
    double fnum = -1337.4201;
    toStringFloat(fnum, numstr_buff, 4);
    terminal_print(numstr_buff);
}

static void scan_keys() {
    terminal_print("press Key Up...\n");
    keyUp = keyboard_scan_key();
    keyboard_scan_key(); // for key release
    toString(keyUp, numstr_buff);
    terminal_print("scan number for Key Up: ");
    terminal_print(numstr_buff);
    
    terminal_print("\npress Key Down...\n");
    keyDown = keyboard_scan_key();
    keyboard_scan_key(); // for key release
    toString(keyDown, numstr_buff);
    terminal_print("scan number for Key Down: ");
    terminal_print(numstr_buff);
    
    terminal_print("\npress Escape...\n");
    keyEsc = keyboard_scan_key();
    keyboard_scan_key(); // for key release
    toString(keyEsc, numstr_buff);
    terminal_print("scan number for Escape: ");
    terminal_print(numstr_buff);
}

static void print_ram() {
    terminal_clear();
    terminal_print("RAM addr | Values:");
    terminal_gotoxy(0, 1);
    for (int row = 0; row < 20; row++) {
        toStringHex(mem_addr + 4 * row * 4, numstr_buff);
        terminal_print(numstr_buff);
        terminal_writechar('|');

        for (int col = 0; col < 4; col++) {
            uint32_t mem_val = ((uint32_t*)(mem_addr + 4 * col + 4 * row * 4))[0];
            toStringHex(mem_val, numstr_buff);
            
            terminal_gotoxy(13 + 12 * col, 1 + row);
            terminal_print(numstr_buff);
            
            terminal_gotoxy(63 + col * 4, 1 + row);
            terminal_putchar((mem_val >> 0) & 0xff);
            terminal_gotoxy(64 + col * 4, 1 + row);
            terminal_putchar((mem_val >> 8) & 0xff);
            terminal_gotoxy(65 + col * 4, 1 + row);
            terminal_putchar((mem_val >> 16) & 0xff);
            terminal_gotoxy(66 + col * 4, 1 + row);
            terminal_putchar((mem_val >> 24) & 0xff);
        }
        terminal_writechar('\n');
    }
    terminal_print("\nstack ptr = ");
    toStringHex((int)stack_ptr, numstr_buff);
    terminal_print(numstr_buff);

}
