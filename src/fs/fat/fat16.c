#include "fat16.h"
#include "memory/memory.h"
#include "config.h"
#include "kernel.h"
#include "disk/disk.h"
#include "disk/streamer.h"
#include "file.h"
#include "memory/heap/kheap.h"
#include "pparser.h"
#include "status.h"
#include "terminal/terminal.h"
#include "whitelib/string.h"
#include <stdint.h>

#define WHITEOS_FAT16_SIGNATURE 0x29
#define WHITEOS_FAT16_FAT_ENTRY_SIZE 0x02
#define WHITEOS_FAT16_BAD_SECTOR 0xFF7
#define WHITEOS_FAT16_UNUSED 0x00

// for whiteos
typedef unsigned int FAT_ITEM_TYPE;
#define FAT_ITEM_TYPE_DIRECTORY 0
#define FAT_ITEM_TYPE_FILE 1

// Fat directory entry attributes bitmask
#define FAT_FILE_READ_ONLY  0x01
#define FAT_FILE_HIDDEN     0x02
#define FAT_FILE_SYSTEM     0x04
#define FAT_FILE_VOLUME_LABEL   0x08
#define FAT_FILE_SUBDIRECTORY   0x10
#define FAT_FILE_ARCHIVED   0x20
#define FAT_FILE_DEVIDE     0x40
#define FAT_FILE_RESERVED   0x80

struct fat_header
{
    uint8_t short_jmp_ins[3];
    uint8_t oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_copies;
    uint16_t root_dir_entries;
    uint16_t number_of_sectors;
    uint8_t media_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t sectors_big;
} __attribute__((packed));

struct fat_header_extended
{
    uint8_t drive_number;
    uint8_t win_nt_bit;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_id_string[11];
    uint8_t system_id_string[8];
} __attribute__((packed));

struct fat_h
{
    struct fat_header primary_header;
    struct fat_header_extended extended_header;
};

struct fat_directory_item
{
    uint8_t filename[8];
    uint8_t ext[3];
    uint8_t attribute;
    uint8_t reserved;
    uint8_t creation_time_tenths_of_a_sec;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access;
    uint16_t high_16_bits_first_cluster;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint16_t low_16_bits_first_cluster;
    uint32_t filesize;
} __attribute__((packed));

struct fat_directory
{
    struct fat_directory_item* items;
    int curr_item;
    int total_items;
    int sector_pos;
    int ending_sector_pos;
};

struct fat_item
{
    struct fat_directory_item* item;
    struct fat_directory* directory;

    FAT_ITEM_TYPE type;
};

// fopen (fat16) result
struct fat_file_descriptor
{
    struct fat_item* item;
    uint32_t pos;
};

//attached to the disk during initialization
struct fat_private
{
    struct fat_h header;
    struct fat_directory root_directory;

    // for data clusters streams
    struct disk_stream* cluster_read_stream;
    // for fat stream
    struct disk_stream* fat_read_stream;
    // for directories
    struct disk_stream* directory_stream;
};

int fat16_resolve(struct disk* disk);
void* fat16_open(struct disk* disk, struct path_part* path, FILE_MODE mode);
int fat16_close(void* private);
//void* fat16_opendir(struct disk* disk, struct path_part* path);
int fat16_seek(void* private, uint32_t offset, FILE_SEEK_MODE seek_mode);
int fat16_read(struct disk* disk, void* descriptor, uint32_t size, uint32_t nmemb, char* out_ptr);
int fat16_readdir(struct disk* disk, void* private, void** entry_private);
int fat16_stat(struct disk* disk, void* private, struct file_stat* stat);

struct filesystem fat16_fs =
{
    .resolve = fat16_resolve,
    .open = fat16_open,
    .close = fat16_close,
    //.opendir = fat16_opendir,
    .read = fat16_read,
    .readdir = fat16_readdir,
    .seek = fat16_seek,
    .stat = fat16_stat
};

struct filesystem* fat16_init()
{
    strcpy(fat16_fs.name, "FAT16");
    return &fat16_fs;
}

static void fat16_init_private(struct disk* disk, struct fat_private* private)
{
    memset(private, 0, sizeof(struct fat_private));
    private->cluster_read_stream = diskstreamer_new(disk->id);
    private->fat_read_stream = diskstreamer_new(disk->id);
    private->directory_stream = diskstreamer_new(disk->id);
}

int fat16_sector_to_absolute(struct disk* disk, int sector)
{
    return sector * disk->sector_size;
}

int fat16_get_total_items_in_directory(struct disk* disk, uint32_t directory_start_sector)
{
    struct fat_directory_item item;
    // struct fat_directory_item empty_item;
    // memset(&empty_item, 0, sizeof(struct fat_directory_item));

    struct fat_private* fat_private = disk->fs_private;

    int res = 0;
    int directory_start_pos = fat16_sector_to_absolute(disk, directory_start_sector);
    struct disk_stream* stream = fat_private->directory_stream;
    if(diskstreamer_seek(stream, directory_start_pos) != WHITEOS_ALL_OK)
    {
        res = -EIO;
        goto out;
    }

    int i = 0;
    // can read max 1 cluster
    while(1)
    {
        if (diskstreamer_read(stream, &item, sizeof(item)) != WHITEOS_ALL_OK)
        {
            res = -EIO;
            goto out;
        }
        if (item.filename[0] == 0x00)
        {
            // empty
            break;
        }
        if (item.filename[0] == 0xE5)
        {
            // unused
            continue;
        }
        i++;
    }
    res = i;
out:
    return res;
}

// sets given fat_directory with root directory data
int fat16_get_root_directory(struct disk* disk, struct fat_private* fat_private, struct fat_directory* directory)
{
    int res = 0;
    struct fat_header* primary_header = &fat_private->header.primary_header;
    // root position in the fat table
    int root_dir_sector_pos = (primary_header->fat_copies * primary_header->sectors_per_fat) + primary_header->reserved_sectors;
    int root_dir_items = fat_private->header.primary_header.root_dir_entries;
    int root_dir_size = root_dir_items * sizeof(struct fat_directory_item);

    int total_items = fat16_get_total_items_in_directory(disk, root_dir_sector_pos);

    struct fat_directory_item* items = kzalloc(root_dir_size);
    if (!items)
    {
        res = -ENOMEM;
        goto out;
    }
    struct disk_stream* stream = fat_private->directory_stream;
    if (diskstreamer_seek(stream, fat16_sector_to_absolute(disk, root_dir_sector_pos)) != WHITEOS_ALL_OK)
    {
        res = -EIO;
        goto out;
    }

    if (diskstreamer_read(stream, items, total_items * sizeof(struct fat_directory_item)) != WHITEOS_ALL_OK)
    {
        res = -EIO;
        goto out;
    }

    directory->items = items;
    directory->total_items = total_items;
    directory->sector_pos = root_dir_sector_pos;
    directory->ending_sector_pos = root_dir_sector_pos + (root_dir_size / disk->sector_size);
    char numstr_buff[32];
    toStringHex(directory->sector_pos * 512, numstr_buff);
    terminal_print("root pos = ");
    terminal_print(numstr_buff);
    terminal_writechar('\n');

    toStringHex(directory->ending_sector_pos * 512, numstr_buff);
    terminal_print("root end = ");
    terminal_print(numstr_buff);
    terminal_writechar('\n');

    toStringHex((directory->ending_sector_pos + ((0 - 2) * primary_header->sectors_per_cluster)) * 512, numstr_buff);
    terminal_print("cluster 0 = ");
    terminal_print(numstr_buff);
    terminal_writechar('\n');

    

out:
    return res;
}

// this function aswers the question "is this fat16 filesystem"? 0 = yes
int fat16_resolve(struct disk* disk)
{
    int res = 0;
    struct fat_private* fat_private = kzalloc(sizeof(struct fat_private));
    fat16_init_private(disk, fat_private);

    disk->fs_private = fat_private;
    disk->filesystem = &fat16_fs;

    struct disk_stream* stream = diskstreamer_new(disk->id);
    if (!stream)
    {
        res = -ENOMEM;
        goto out;
    }
    if (diskstreamer_read(stream, &fat_private->header, sizeof(fat_private->header)) != WHITEOS_ALL_OK)
    {
        res = -EIO;
        goto out;
    }

    if (fat_private->header.extended_header.signature != 0x29)
    {
        res = -EFSNOTUS;
        goto out;
    }

    if (fat16_get_root_directory(disk, fat_private, &fat_private->root_directory) != WHITEOS_ALL_OK)
    {
        res = -EIO;
        goto out;
    }
    terminal_print("Disk ");
    terminal_writechar('0' + disk->id);
    terminal_print(":/ has FAT16 filesystem\n");

out:
    if (stream)
    {
        diskstreamer_close(stream);
    }
    if (res < 0)
    {
        kfree(fat_private);
        disk->fs_private = 0;
    }
    return res;
}

// fat16 uses spaces as string terminators
// use separately for filenames and extensions
void fat16_to_proper_string(char** out, const char* in)
{
    while(*in != '\0' && *in != ' ')
    {
        **out = *in;
        *out += 1;
        in += 1;
    }
    if (*in == ' ') {
        **out = '\0';
    }
}

void fat16_get_full_relative_filename(struct fat_directory_item* item, char* out, int max_len)
{
    memset(out, 0x00, max_len);
    char* out_tmp = out;
    fat16_to_proper_string(&out_tmp, (const char*) item->filename);

    // if file has extention
    if (item->ext[0] != '\0' && item->ext[0] != ' ')
    {
        *out_tmp++ = '.';
        fat16_to_proper_string(&out_tmp, (const char*) item->ext);
    }
}

struct fat_directory_item* fat16_clone_directory_item(struct fat_directory_item* item, int size)
{
    struct fat_directory_item* item_copy = 0;
    if (size < sizeof(struct fat_directory_item))
    {
        goto out;
    }
    item_copy = kzalloc(size);
    if (!item_copy)
    {
        goto out;
    }
    memcpy(item_copy, item, size);
out:
    return item_copy;
}

static uint32_t fat16_get_first_cluster(struct fat_directory_item* item)
{
    return (item->high_16_bits_first_cluster /* << 16 (?)*/) | (item->low_16_bits_first_cluster);
}

static int fat16_cluster_to_sector(struct fat_private* private, int cluster)
{
    return private->root_directory.ending_sector_pos + ((cluster - 2) * private->header.primary_header.sectors_per_cluster);
}

// first allocation table comes right after reserved sectors;
static uint32_t fat16_get_first_fat_sector(struct fat_private* private)
{
    return private->header.primary_header.reserved_sectors;
}

// returns the value from FAT table for a given cluster
static int fat16_get_fat_entry(struct disk* disk, int cluster)
{
    int res = -1;
    struct fat_private* private = disk->fs_private;
    struct disk_stream* stream = private->fat_read_stream;
    if (!stream)
        goto out;

    uint32_t fat_table_position = fat16_get_first_fat_sector(private) * disk->sector_size;
    res = diskstreamer_seek(stream, fat_table_position * (cluster * WHITEOS_FAT16_FAT_ENTRY_SIZE));
    if (res < 0)
        goto out;

    uint16_t entry = 0;
    res = diskstreamer_read(stream, &entry, sizeof(entry));
    if (res < 0)
        goto out;

    res = entry;
out:
    return res;
}

// go through the clusters chain in FAT table. Used for large files that have multiple clusters
// offset in Bytes.
static int fat16_get_cluster_for_offset(struct disk* disk, int starting_cluster, int offset)
{
    int res = 0;
    struct fat_private* private = disk->fs_private;
    int size_of_cluster_bytes = private->header.primary_header.sectors_per_cluster * disk->sector_size;
    int cluster_to_use = starting_cluster;
    int clusters_ahead = offset / size_of_cluster_bytes;
    for (int i = 0; i < clusters_ahead; i++)
    {
        int entry = fat16_get_fat_entry(disk, cluster_to_use);
        // last entry, but expected more.
        if(entry == 0xFF8 || entry == 0xFFF)
        {
            res = -EIO;
            goto out;
        }

        // is sector marked bad?
        if (entry == WHITEOS_FAT16_BAD_SECTOR)
        {
            res = -EIO;
            goto out;
        }

        // reserved sectors?
        if (entry == 0xFF0 || entry == 0xFF6)
        {
            res = -EIO;
            goto out;
        }

        // there was no cluster?
        if (entry == 0x00)
        {
            res = -EIO;
            goto out;
        }
        cluster_to_use = entry;
    }

    res = cluster_to_use;
out:
    return res;

}

//
static int fat16_read_internal_from_stream(struct disk* disk, struct disk_stream* stream, int cluster, int offset, int total, void* out)
{
    int res = 0;
    struct fat_private* private = disk->fs_private;
    int size_of_cluster_bytes = private->header.primary_header.sectors_per_cluster * disk->sector_size;
    // calculate which cluster to read
    int cluster_to_use = fat16_get_cluster_for_offset(disk, cluster, offset);
    if (cluster_to_use < 0)
    {
        res = cluster_to_use;
        goto out;
    }
    // calculate the starting position inside the cluter
    int offset_from_cluster = offset % size_of_cluster_bytes;
    int starting_sector = fat16_cluster_to_sector(private, cluster_to_use);
    int starting_pos = (starting_sector * disk->sector_size) + offset_from_cluster;
    // can read max 1 cluster at a time because they can be scattered around the disk
    // bug: if offset is >0 and wanna read full cluster then it will start reading cluster next to it
    // sizeo_of_cluster_bytes - offset_from_cluster
    int total_to_read = (total > size_of_cluster_bytes) ? size_of_cluster_bytes : total;
    res = diskstreamer_seek(stream, starting_pos);
    if (res != WHITEOS_ALL_OK)
        goto out;
    res = diskstreamer_read(stream, out, total_to_read);
    if (res != WHITEOS_ALL_OK)
        goto out;

    total -= total_to_read;
    // read another cluster if needed
    if (total > 0)
        res = fat16_read_internal_from_stream(disk, stream, cluster, offset + total_to_read, total, out + total_to_read);
out:
    return res;
}

static int fat16_read_internal(struct disk* disk, int starting_cluster, int offset, int total, void* out)
{
    struct fat_private* fs_private = disk->fs_private;
    struct disk_stream* stream = fs_private->cluster_read_stream;
    return fat16_read_internal_from_stream(disk, stream, starting_cluster, offset, total, out);
}

void fat16_free_directory(struct fat_directory* directory)
{
    if(!directory) return;

    // assert here
    if (directory->items)
        kfree(directory->items);
    kfree(directory);
}

void fat16_fat_item_free(struct fat_item* item)
{
    if (item->type == FAT_ITEM_TYPE_DIRECTORY)
        fat16_free_directory(item->directory);
    kfree(item->item);
    kfree(item);
}

int fat16_isdir(struct fat_directory_item* item)
{
    return (item->attribute & FAT_FILE_SUBDIRECTORY);
}

struct fat_directory* fat16_load_fat_directory(struct disk* disk, struct fat_directory_item* item)
{
    int res = 0;
    struct fat_directory* directory = 0;
    struct fat_private* fat_private = disk->fs_private;

    // ignore files
    if (!fat16_isdir(item))
    {
        res = -EINVARG;
        goto out;
    }

    directory = kzalloc(sizeof(struct fat_directory));
    if (!directory)
    {
        res = -ENOMEM;
        goto out;
    }

    // cluster where directory items (files/folders) are listed
    int cluster = fat16_get_first_cluster(item);
    int cluster_sector = fat16_cluster_to_sector(fat_private, cluster);
    // read from disk how many files/folders are in directory (could save those entries on the way)
    int total_items = fat16_get_total_items_in_directory(disk, cluster_sector);
    directory->total_items = total_items;
    int directory_size = directory->total_items * sizeof(struct fat_directory_item);
    // loading full directory to the heap
    directory->items = kzalloc(directory_size);
    if (!directory->items)
    {
        res = -ENOMEM;
        goto out;
    }
    // reads the data from disk for the second time... and with the different diskstream...
    res = fat16_read_internal(disk, cluster, 0x00, directory_size, directory->items);
    if (res != WHITEOS_ALL_OK)
    {
        goto out;
    }
    directory->curr_item = 0;
out:
    if (res != WHITEOS_ALL_OK)
    {
        fat16_free_directory(directory);
    }
    return directory;
}

struct fat_item* fat16_new_fat_item_for_directory_item(struct disk* disk, struct fat_directory_item* item)
{
    struct fat_item* f_item = kzalloc(sizeof(struct fat_item));
    if (!f_item)
    {
        return 0;
    }

    f_item->item = fat16_clone_directory_item(item, sizeof(struct fat_directory_item));
    if (fat16_isdir(item))
    {
        // if it's a folder then load the folder contents
        f_item->directory = fat16_load_fat_directory(disk, item);
        f_item->type = FAT_ITEM_TYPE_DIRECTORY;
    }
    else
    {
        f_item->type = FAT_ITEM_TYPE_FILE;
    }

    return f_item;
}

struct fat_item* fat16_find_item_in_directory(struct disk* disk, struct fat_directory* directory, const char* name)
{
    struct fat_item* f_item = 0;
    char tmp_filename[WHITEOS_MAX_PATH];
    for (int i = 0; i < directory->total_items; i++)
    {
        fat16_get_full_relative_filename(&directory->items[i], tmp_filename, sizeof(tmp_filename));
        if (istrncmp(tmp_filename, name, sizeof(tmp_filename)) == 0)
        {
            // found matching filename.
            f_item = fat16_new_fat_item_for_directory_item(disk, &directory->items[i]);
            break;
        }
    }
    return f_item;
}

struct fat_item* fat16_get_directory_entry(struct disk* disk, struct path_part* path)
{
    struct fat_private* fat_private = disk->fs_private;
    struct fat_item* current_item = 0;
    struct fat_item* root_item = fat16_find_item_in_directory(disk, &fat_private->root_directory, path->name);
    if (!root_item)
    {
        terminal_print("NO ROOT\n");
        goto out;
    }

    struct path_part* next_part = path->next;
    current_item = root_item;

    while (next_part != 0)
    {
        if (current_item->type != FAT_ITEM_TYPE_DIRECTORY)
        {
            terminal_print("trying to use file as directory\n");
            current_item = 0;
            break;
        }
        // char tmpname[129];
        // fat16_get_full_relative_filename(current_item->item, tmpname, 129);
        // terminal_print("current: \n\tname: ");
        // terminal_print(tmpname);
        // terminal_print("\n\ttype: ");
        // terminal_writechar('0' + current_item->type);
        // terminal_writechar('\n');
        struct fat_item* tmp_item = fat16_find_item_in_directory(disk, current_item->directory, next_part->name);
        fat16_fat_item_free(current_item);
        current_item = tmp_item;
        next_part = next_part->next;
    }
out:
    return current_item;
}

static void fat16_free_file_descriptor(struct fat_file_descriptor* desc)
{
    fat16_fat_item_free(desc->item);
    kfree(desc);
}

void* fat16_open(struct disk* disk, struct path_part* path, FILE_MODE mode)
{
    // if (mode != FILE_MODE_READ)
    //     return ERROR(-ERDONLY);

    struct fat_file_descriptor* descriptor = 0;
    descriptor = kzalloc(sizeof(struct fat_file_descriptor));
    if (!descriptor)
    {
        terminal_print("no descriptor\n");
        return ERROR(-ENOMEM);
    }
    if (!path)
    {
        // open root
        descriptor->item = kzalloc(sizeof(struct fat_item));
        descriptor->item->directory = &((struct fat_private*)disk->fs_private)->root_directory;
        descriptor->item->type = FAT_ITEM_TYPE_DIRECTORY;
    }
    else descriptor->item = fat16_get_directory_entry(disk, path);
    if (!descriptor->item)
    {
        fat16_free_file_descriptor(descriptor);
        terminal_print("no descriptor item\n");
        return ERROR(-EIO);
    }
    // else if (descriptor->item->type != FAT_ITEM_TYPE_FILE)
    // {
    //     fat16_free_file_descriptor(descriptor);
    //     terminal_print("this is not a file\n");
    //     return ERROR(-EIO);
    // }

    descriptor->pos = 0;
    // if (descriptor->item->type == FAT_ITEM_TYPE_DIRECTORY) {
    //     terminal_print("its a dir\n");
    //     if (descriptor->item->directory->items[0].filename[0] == 0x00)
    //         terminal_print("empty\n");
    // }
    // else
    //     terminal_print("aint a dir\n");
    return descriptor;
}

int fat16_close(void* private)
{
    fat16_free_file_descriptor((struct fat_file_descriptor*)private);
    return 0;
}

int fat16_read(struct disk* disk, void* descriptor, uint32_t size, uint32_t nmemb, char* out_ptr)
{
    int res = 0;

    struct fat_file_descriptor* fat_desc = descriptor;
    struct fat_item* item = fat_desc->item;
    int offset = fat_desc->pos;

    for (uint32_t i = 0; i < nmemb; i++)
    {
        res = fat16_read_internal(disk, fat16_get_first_cluster(item->item), offset, size, out_ptr);
        if (ISERR(res))
            goto out;
        out_ptr += size;
        offset += size;
    }
    res = nmemb;

out:
    return res;
}

int fat16_stat(struct disk* disk, void* private, struct file_stat* stat)
{
    int res = 0;
    struct fat_file_descriptor* descriptor = (struct fat_file_descriptor*) private;
    struct fat_item* item = descriptor->item;
    if (item->type != FAT_ITEM_TYPE_FILE)
    {
        res = -EINVARG;
        goto out;
    }

    struct fat_directory_item* ritem = item->item;
    stat->filesize = ritem->filesize;
    stat->flags = 0x00;
    
    if (ritem->attribute & FAT_FILE_READ_ONLY)
    {
        stat->flags |= FILE_STAT_READ_ONLY;
    }

out:
    return res;
}

int fat16_seek(void* private, uint32_t offset, FILE_SEEK_MODE seek_mode)
{
    int res = 0;
    struct fat_file_descriptor* desc = private;

    struct fat_item* item = desc->item;
    if (item->type != FAT_ITEM_TYPE_FILE)
    {
        res = -EINVARG;
        goto out;
    }

    struct fat_directory_item* ritem = item->item;
    switch (seek_mode)
    {
        case SEEK_SET:
            if (offset >= ritem->filesize)
            {
                res = -EIO;
                goto out;
            }
            desc->pos = offset;
        break;

        case SEEK_END:
            res = -EUNIMP;
        break;

        case SEEK_CUR:
            if (desc->pos + offset >= ritem->filesize)
            {
                res = -EIO;
                goto out;
            }
            desc->pos += offset;
        break;

        default: res = -EINVARG;
    }
out:
    return res;
}

int fat16_readdir(struct disk* disk, void* private, void** entry_private)
{
    int res = 0;
    struct fat_file_descriptor* desc = private;
    
    if (desc->item->type != FAT_ITEM_TYPE_DIRECTORY)
    {
        res = -EINVARG;
        goto out;
    }

    struct fat_directory* dir = desc->item->directory;
    // reached last entry
    if (dir->curr_item >= dir->total_items)
    {
        res = 1;
        goto out;
    }
    *entry_private = &dir->items[dir->curr_item];
    dir->curr_item++;
out:
    return res;
}