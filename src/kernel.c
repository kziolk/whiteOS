#include <stddef.h>
#include <stdint.h>

#include "idt/idt.h"
#include "io/io.h"
#include "whitelib/system.h"
#include "terminal/terminal.h"
#include "keyboard/keyboard.h"
#include "whitelib/string.h"
#include "whitelib/programs/ramscroller.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "disk/disk.h"

static char numstr_buff[32];

static struct paging_4gb_chunk* kernel_chunk = 0;
void kernel_main(void* multiboot_info_ptr, uint32_t magic_number) 
{
    terminal_clear();
    // initialize the heap
    kheap_init();
    
    // initialize the interrupt descriptor table
    idt_init();
    // works without
    // kb_init();
    
    // setup paging
    kernel_chunk =  paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCES_FROM_ALL);
    // switch to kernel paging chunk
    paging_switch(paging_4gb_chunk_get_directory(kernel_chunk));
    enable_paging();

    enable_interrupts();
    div_by_zero();
    
    system_events_init();
    
    // test paging, 
    char yeah[] = "this is text written on stack so i can now move it to the pointer";
    char* ptr = kzalloc(4096);
    int i = 0;
    while(yeah[i]) {
        ptr[i] = yeah[i];
        i++;
    }
    
    paging_set(paging_4gb_chunk_get_directory(kernel_chunk), (void*)0x1000, (uint32_t)ptr | PAGING_ACCES_FROM_ALL | PAGING_IS_PRESENT | PAGING_IS_WRITEABLE);
    disk_search_and_init();    
    struct disk* kdisk = disk_get(0);
    int read_res = disk_read_block(kdisk, 0, 1, ptr);
    if (read_res) return;   
    toString((int)ptr[0], numstr_buff);
    terminal_print("disk read out: ");
    terminal_print(numstr_buff);


    //ramscroller();
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
