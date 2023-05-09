#pragma once

// Representation of the file in the system (VFS)
#include "disk/disk.h"
#include "pparser.h"
#include <stdint.h>

typedef unsigned int FILE_SEEK_MODE;
enum {
    SEEK_SET,
    SEEK_CUR,
    SEEK_END
};

typedef unsigned int FILE_MODE;
enum {
    FILE_MODE_READ,
    FILE_MODE_WRITE,
    FILE_MODE_APPEND,
    FILE_MODE_INVALID
};


typedef unsigned int FILE_STAT_FLAGS;
enum {
    FILE_STAT_READ_ONLY = 0b00000001
};

struct disk;
struct file_stat;
// usage: pass disk, path_part root node, and file mode
typedef void*(*FS_OPEN_FUNCTION)(struct disk* disk, struct path_part* path, FILE_MODE mode);
typedef int(*FS_CLOSE_FUNCTION)(void* private);
typedef int(*FS_READ_FUNCTION)(struct disk* disk, void* private, uint32_t size, uint32_t nmemb, char* out);
typedef int(*FS_SEEK_FUNCTION)(void* private, uint32_t offset, FILE_SEEK_MODE seek_mode);
typedef int(*FS_STAT_FUNCTION)(struct disk* disk, void* private, struct file_stat* stat);

// open directory too.
typedef void*(*FS_OPEN_DIR_FUNCTION)(struct disk* disk, struct path_part* path);

// file system returns a flag telling whether it can handle the disk
typedef int (*FS_RESOLVE_FUNCTION)(struct disk* disk);

struct filesystem
{
    FS_RESOLVE_FUNCTION resolve;
    FS_OPEN_FUNCTION open;
    FS_CLOSE_FUNCTION close;
    FS_OPEN_DIR_FUNCTION opendir;
    FS_READ_FUNCTION read;
    FS_SEEK_FUNCTION seek;
    FS_STAT_FUNCTION stat;

    char name[20];
};

// file entry in whiteOS
struct file_descriptor
{
    int index;
    struct filesystem* filesystem;

    // private data of file system
    void* private;

    struct disk* disk;
};

struct file_stat
{
    FILE_STAT_FLAGS flags;
    uint32_t filesize;
};

void fs_init();
int fopen(const char* filename, const char* mode_str);
int fclose(int fd);
// int opendir(const char* filename);
int fread(void* ptr, uint32_t size, uint32_t nmemb, int fd);
int fstat(int fd, struct file_stat* stat);
int fseek(int fd, int offset, FILE_SEEK_MODE whence);
void fs_insert_filesystem(struct filesystem* filesystem);
struct filesystem* fs_resolve(struct disk* disk);
struct file_descriptor* file_get_descriptor(int fd);
