#pragma once

#include "fs/file.h"

typedef unsigned int WHITEOS_DISK_TYPE;

// Represents a real physical hard disk
#define WHITEOS_DISK_TYPE_REAL 0

struct disk
{
    WHITEOS_DISK_TYPE type;
    int sector_size;

    int id;

    struct filesystem* filesystem;
    void* fs_private;
};

int disk_read_block(struct disk* idisk, unsigned int lba, int total, void* buf);
int disk_write_block(struct disk* idisk, unsigned int lba, int total, void* buf);
struct disk* disk_get(int index);

void disk_search_and_init();

