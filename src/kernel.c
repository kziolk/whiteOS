#include <stddef.h>
#include <stdint.h>

#include "idt/idt.h"
#include "io/io.h"
#include "whitelib/system.h"
#include "terminal/terminal.h"
#include "keyboard/keyboard.h"
#include "whitelib/string.h"
#include "whitelib/programs/ramscroller.h"
#include "whitelib/programs/fileexplorer.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "disk/disk.h"

// tmp char buffer for int to string conversions
static char numstr_buff[32];
// page directory for paging
static struct paging_4gb_chunk* kernel_chunk = 0;

void test_paging();

void kernel_main(void* multiboot_info_ptr, uint32_t magic_number)
{
    terminal_clear();
    // initialize the heap
    kheap_init();

    // search and initialize the disk
    disk_search_and_init();

    // initialize the interrupt descriptor table
    idt_init();
    // keyboard works without
    // kb_init();

    // setup paging
    kernel_chunk = paging_new_4gb(PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCES_FROM_ALL);
    // switch to kernel paging chunk
    paging_switch(paging_4gb_chunk_get_directory(kernel_chunk));
    enable_paging();

    // Enable interrupts here.
    enable_interrupts();
    //div_by_zero();

    // non-handbook experimenting with some system events
    system_events_init();

    // test paging,
    test_paging();

    // run interactive programs
    ramscroller();
    fileexplorer();


    // some idea for handling exceptions after implementing multiprocessing
    /*while (1) {
        int exception = fetchException();
        if (exception) {
            char exceptionStr[] = { '0' + exception, '\n', 0 };
            terminal_print("exception ");
            terminal_print(exceptionStr);
        }
    }*/

    terminal_clear();
    terminal_print("\nexitting kernel...\npress any key to shutdown...");
    keyboard_scan_key();
    keyboard_scan_key();
    outw(0x604, 0x2000);
}

void test_paging()
{
    char yeah[] = "Text on the heap!";
    char* ptr = kzalloc(4096);
    int i = 0;
    // copy text to pointer allocadet on heap
    while(yeah[i]) {
        ptr[i] = yeah[i];
        i++;
    }
    uint32_t* page_directory = paging_4gb_chunk_get_directory(kernel_chunk);
    // set pointer address to 0x1000 in kernel page directory
    // works only on addresses aligned to 2^12 for paging entry flags (kernel malloc compatible)
    paging_set(page_directory, (void*)0x1000, (intptr_t)ptr | PAGING_ACCES_FROM_ALL | PAGING_IS_PRESENT | PAGING_IS_WRITEABLE);

    // also a little ata lba disk read test
    struct disk* disk_primary = disk_get(0);
    // read first sector containing boot loader
    int read_output = disk_read_block(disk_primary, 0, 1, ptr + 32);
    if (read_output) {
        terminal_print("kernel init test - disk_read_block returned: ");
        toString(read_output, numstr_buff);
        terminal_print(numstr_buff);
        terminal_print("\ninit not successfull");
        while(1);
    }
}
