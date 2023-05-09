#include "disk.h"
#include "fs/file.h"
#include "io/io.h"
#include "memory/memory.h"
#include "config.h"
#include "status.h"
#include "terminal/terminal.h"

struct disk disk;

static int disk_read_sector(int lba, int total, void* buf)
{
    outb(0x1F6, (lba >> 24) | 0xE0);
    outb(0x1F2, total);
    outb(0x1F3, (unsigned char)(lba & 0xFF));
    outb(0x1F4, (unsigned char)(lba >> 8));
    outb(0x1F5, (unsigned char)(lba >> 16));
    outb(0x1F7, 0x20);

    unsigned short* ptr = (unsigned short*) buf;
    for (int b = 0; b < total; b++)
    {
        // wait for the buffer
        char c = insb(0x1F7);
        while(!(c & 0x08))
            c = insb(0x1F7);

        // ready to read from ata controller
        // copy from hard disk to ram
        for (int i = 0; i < 256; i++)
        {
            *ptr = insw(0x1F0);
            ptr++;
        }
    }
        // thats it

    return 0;
}

static int disk_write_sector(int lba, int total, void* buf)
{
    outb(0x1F6, (lba >> 24) | 0xE0);
    outb(0x1F2, total);
    outb(0x1F3, (unsigned char)(lba & 0xFF));
    outb(0x1F4, (unsigned char)(lba >> 8));
    outb(0x1F5, (unsigned char)(lba >> 16));
    outb(0x1F7, 0x30);

    unsigned short* ptr = (unsigned short*) buf;
    for (int b = 0; b < total; b++)
    {
        // wait for the buffer
        char c = insb(0x1F7);
        while(!(c & 0x08))
            c = insb(0x1F7);

        // write word by word
        for (int i = 0; i < 256; i++)
        {
            outw(0x1F0, *ptr);
            ptr++;
        }

    }
    return 0;
}

void disk_search_and_init()
{
    // found real current disk
    memset(&disk, 0, sizeof(disk));
    disk.type = WHITEOS_DISK_TYPE_REAL;
    disk.sector_size = WHITEOS_SECTOR_SIZE;
    disk.id = 0; // hardcode 1 primary disk with id = 0
    disk.filesystem = fs_resolve(&disk);
    if (!disk.filesystem)
        terminal_print("Disk 0:/ has no filesystem\n");
}

struct disk* disk_get(int index)
{
    if (index != 0)
        return 0;
    return &disk;
}

int disk_write_block(struct disk* idisk, unsigned int lba, int total, void* buf)
{
    if (idisk != &disk)
        return -EIO;

    return disk_write_sector(lba, total, buf);
}

int disk_read_block(struct disk* idisk, unsigned int lba, int total, void* buf)
{
    if (idisk != &disk)
        return -EIO;

    return disk_read_sector(lba, total, buf);
}
