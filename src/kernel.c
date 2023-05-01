#include <stddef.h>
#include <stdint.h>

#include "idt/idt.h"
#include "io/io.h"
#include "whitelib/system.h"
#include "terminal/terminal.h"
#include "keyboard/keyboard.h"
#include "whitelib/programs/defscreen.h"
#include "memory/heap/kheap.h"

void kernel_main(void* multiboot_info_ptr, uint32_t magic_number) 
{
    terminal_clear();
    idt_init();
    //kb_init();
    enable_interrupts();
    div_by_zero();
    //div_by_zero();
    
    kheap_init();
    system_events_init();
    defscreen_start();
    /*while (1) {
        int exception = fetchException();
        if (exception) {
            char exceptionStr[] = { '0' + exception, '\n', 0 };
            terminal_print("exception ");
            terminal_print(exceptionStr);
        }
    }*/
    

    terminal_print("\nexitting kernel...\npress any key to shutdown...");
    keyboard_scan_key();
    keyboard_scan_key();
    outw(0x604, 0x2000);
}
