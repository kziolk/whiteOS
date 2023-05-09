#include "file.h"
#include "kernel.h"
#include "config.h"
#include "fat/fat16.h"
#include "memory/heap/kheap.h"
#include "status.h"
#include "memory/memory.h"
#include "terminal/terminal.h"
#include "whitelib/string.h"

struct filesystem* filesystems[WHITEOS_MAX_FILESYSTEMS];
struct file_descriptor* file_descriptors[WHITEOS_MAX_FILE_DESCRIPTORS];

// returns address to filesystem entry
static struct filesystem** fs_get_free_filesystem_entry()
{
    for (int i = 0; i < WHITEOS_MAX_FILESYSTEMS; i++)
        if (filesystems[i] == 0)
            return &filesystems[i];
    // whiteOS has initialized max filesystems, no space for another one
    return 0;
}

// filesystem drivers can insert their functionality to the filesystems entry
void fs_insert_filesystem(struct filesystem* filesystem)
{
    if (filesystem == 0)
    {
        // panic
        while(1){}
    }

    struct filesystem** fs = fs_get_free_filesystem_entry();
    if (!fs)
    {
        // panic
        while(1){}
    }
    *fs = filesystem;
}

// load static filesystems implemented in kernel
static void fs_static_load()
{
    fs_insert_filesystem(fat16_init());
}

void fs_load()
{
    fs_static_load();


}

void fs_init()
{
    memset(file_descriptors, 0, sizeof(file_descriptors));
    memset(filesystems, 0, sizeof(filesystems));
    fs_load();
}

static void file_free_descriptor(struct file_descriptor* desc)
{
    file_descriptors[desc->index] = 0x00;
    kfree(desc);
}

// returns 0 if allocated new descriptor
// future improvement: remember descriptors size and don't search for free space.
static int file_new_descriptor(struct file_descriptor** desc_out)
{
    // default response
    int res = -ENOMEM;
    for (int i = 0; i < WHITEOS_MAX_FILE_DESCRIPTORS; i++) {
        if (file_descriptors[i] == 0) {
            struct file_descriptor* desc = kzalloc(sizeof(struct file_descriptor));
            // Descriptors start at 1
            desc->index = i + 1;
            file_descriptors[i] = desc;
            *desc_out = desc;
            res = 0;
            break;
        }
    }
    return res;
}

struct file_descriptor* file_get_descriptor(int fd)
{
    if (fd <= 0 || fd >= WHITEOS_MAX_FILE_DESCRIPTORS)
        return 0;

    // descriptors start at 1
    int index = fd - 1;
    return file_descriptors[index];
}


struct filesystem* fs_resolve(struct disk* disk)
{
    struct filesystem* fs = 0;
    for (int i = 0; i < WHITEOS_MAX_FILESYSTEMS; i++)
    {
        if (filesystems[i] != 0 && filesystems[i]->resolve(disk) == 0) {
            fs = filesystems[i];
            break;
        }
    }
    return fs;
}

FILE_MODE file_get_mode_by_string(const char* str)
{
    FILE_MODE mode = FILE_MODE_INVALID;
    if (strncmp(str, "r", 1) == 0)
        mode = FILE_MODE_READ;
    else if (strncmp(str, "w", 1) == 0)
        mode = FILE_MODE_WRITE;
    else if (strncmp(str, "a", 1) == 0)
        mode = FILE_MODE_APPEND;
    else if (strncmp(str, "dir", 1) == 0)
        mode = FILE_MODE_DIRECTORY;
    return mode;
}

int fopen(const char* filename, const char* mode_str)
{
    int res = 0;
    // parse path
    struct path_root* root_path = pparser_parse(filename, NULL);
    if (!root_path)
    {
        terminal_print("\tno root path");
        res = -EINVARG;
        goto out;
    }

    // // not allowing opening root dir 0:/
    // if (!root_path->first)
    // {
    //     terminal_print("\tno root->first\n");
    //     res = -EINVARG;
    //     goto out;
    // }
    // get the disc and check filesystem on it
    struct disk* disk = disk_get(root_path->drive_no);
    if (!disk)
    {
        terminal_print("\tno disk\n");
        res = -EIO;
        goto out;
    }

    if (!disk->filesystem)
    {
        terminal_print("\tno filesystem\n");
        res = -EIO;
        goto out;
    }
    // get file open mode
    FILE_MODE mode = file_get_mode_by_string(mode_str);
    if (mode == FILE_MODE_INVALID)
    {
        terminal_print("\tinvalid mode\n");
        res = -EINVARG;
        goto out;
    }
    // call open function from filesystem driver
    void* descriptor_private_data = disk->filesystem->open(disk, root_path->first, mode);
    if (ISERR(descriptor_private_data))
    {
        terminal_print("\tprivate data error\n");
        res = ERROR_I(descriptor_private_data);
        goto out;
    }
    // create new file descriptor; associate it with file system and disk.
    struct file_descriptor* desc = 0;
    res = file_new_descriptor(&desc);
    if (res < 0) goto out;
    desc->filesystem = disk->filesystem;
    desc->private = descriptor_private_data;
    desc->disk = disk;
    res = desc->index;

out:
    // todo: handle errors
    if (res < 0) res = 0;
    return res;
}

int fclose(int fd)
{
    int res = 0;
    struct file_descriptor* desc = file_get_descriptor(fd);
    if (!desc)
    {
        res = -EIO;
        goto out;
    }

    res = desc->filesystem->close(desc->private);
    if (res == WHITEOS_ALL_OK)
        file_free_descriptor(desc);
out:


    return res;
}


int fstat(int fd, struct file_stat* stat)
{
    int res = 0;
    struct file_descriptor* desc = file_get_descriptor(fd);
    if (!desc)
    {
        res = -EIO;
        goto out;
    }

    res = desc->filesystem->stat(desc->disk, desc->private, stat);
out:
    return res;
}

int fseek(int fd, int offset, FILE_SEEK_MODE whence)
{
    int res = 0;
    struct file_descriptor* desc = file_get_descriptor(fd);
    if (!desc)
    {
        res = -EIO;
        goto out;
    }

    res = desc->filesystem->seek(desc->private, offset, whence);
out:
    return res;
}

int fread(void* ptr, uint32_t size, uint32_t nmemb, int fd)
{
    int res = 0;
    if (size == 0 || nmemb == 0 || fd < 1)
    {
        res = -EINVARG;
        goto out;
    }

    struct file_descriptor* desc = file_get_descriptor(fd);
    if (!desc)
    {
        res = -EINVARG;
        goto out;
    }

    res = desc->filesystem->read(desc->disk, desc->private, size, nmemb, (char*)ptr);
out:
    return res;
}

int readdir(int fd, struct dirent* entry)
{
    int res = 0;
    struct file_descriptor* desc = file_get_descriptor(fd);
    if (!desc)
    {
        res = -EIO;
        goto out;
    }

    res = desc->filesystem->readdir(desc->disk, desc->private, &entry->private);
    if (res)
        goto out;
    fat16_get_full_relative_filename(entry->private, entry->name, 12);
out:
    return res;
}