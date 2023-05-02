#pragma once

typedef unsigned int WHITEOS_DISK_TYPE;

// Represents a real physical hard disk
#define WHITEOS_DISK_TYPE_REAL 0

struct disk
{
    WHITEOS_DISK_TYPE type;
    int sector_size;
};

int disk_read_block(struct disk* idisk, unsigned int lba, int total, void* buf);
struct disk* disk_get(int index);

void disk_search_and_init();

