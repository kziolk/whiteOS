#include <stddef.h>
#include <stdint.h>

#include "fs/file.h"
#include "idt/idt.h"
#include "io/io.h"
#include "memory/memory.h"
#include "whitelib/system.h"
#include "terminal/terminal.h"
#include "keyboard/keyboard.h"
#include "whitelib/string.h"
#include "whitelib/programs/ramscroller.h"
#include "whitelib/programs/fileexplorer.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "disk/disk.h"
#include "disk/streamer.h"

// tmp char buffer for int to string conversions
static char numstr_buff[32];
// page directory for paging
static struct paging_4gb_chunk* kernel_chunk = 0;

void test_write_lba();
void test_paging();
void test_fat();

void panic(const char* msg)
{
    terminal_print(msg);
    while(1){}
}

void kernel_main(void* multiboot_info_ptr, uint32_t magic_number)
{
    terminal_clear();
    // initialize the heap
    kheap_init();

    // initialize filesystems
    fs_init();

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

    // test ata lba write
    //test_write_lba();

    // test paging,
//    test_paging();

    // FAT 10h
    //test_fat();

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

void test_write_lba()
{
    struct disk* disk_primary = disk_get(0);
    char* ptr = kmalloc(512);
    memset(ptr, 'A', 31);
    ptr[31] = '\0';

    int write_output = disk_write_block(disk_primary, 0, 1, ptr);
    terminal_print("\nwrote ");
    toStringHex((unsigned int)(*ptr), numstr_buff);
    terminal_print(numstr_buff);
    terminal_writechar('\n');
    if (write_output) {
        terminal_print("kernel init test - disk_write_block returned: ");
        toString(write_output, numstr_buff);
        terminal_print(numstr_buff);
    }
    else {
        memset(ptr, 0x00, 512);

        int read_output = disk_read_block(disk_primary, 0, 1, ptr);
        if (read_output) {
            terminal_print("kernel init test - disk_read_block returned: ");
            toString(read_output, numstr_buff);
            terminal_print(numstr_buff);
            terminal_print("\ninit not successfull");
            while(1);
        }
        terminal_print("\nread: ");
        toStringHex((unsigned int)(*ptr), numstr_buff);
        terminal_print(numstr_buff);
        terminal_writechar('\n');
    }
    keyboard_scan_key();
    keyboard_scan_key();
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

    struct disk_stream* stream = diskstreamer_new(0);
    diskstreamer_seek(stream, 0x01);
    unsigned char c = 0;
    diskstreamer_read(stream, &c, 1);
    terminal_print("yes it works.\nstreamer read byte: ");
    toStringHex((int)c, numstr_buff);
    terminal_print(numstr_buff);
    terminal_writechar('\n');
    keyboard_scan_key();
    keyboard_scan_key();
}

void test_fat()
{
    int fd = fopen("0:/script/hello.txt", "r");
    if (fd) {
        terminal_print("\nOpened: ");
        fseek(fd, 2, SEEK_SET);
        fread(numstr_buff, 32, 1, fd);
        terminal_print(numstr_buff);
    } else {
        terminal_print("oops :(\n");
    }
    keyboard_scan_key();
    keyboard_scan_key();
}
