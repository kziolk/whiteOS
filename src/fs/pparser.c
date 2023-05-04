#include "pparser.h"
#include "config.h"
#include "status.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "whitelib/string.h"

static int pparser_path_valid_format(const char* path)
{
    int len = strnlen(path, WHITEOS_MAX_PATH);
    return (len >= 3) && isdigit(path[0]) && (memcmp((void*)&path[1], ":/", 2) == 0);
}

// extracts drive number from path and shifts path pointer to point to the next filename
// e.g. path: "0:/script/snake" -> "script/snake"; return 0
static int pparser_get_drive_from_path(const char** path)
{
    if (!pparser_path_valid_format(*path))
        return -EBADPATH;

    *path += 3;
    return (*path)[-3] - '0';
}

// create root node of the path containing disk number
static struct path_root* pparser_create_root(int drive_no)
{
    struct path_root* root = kmalloc(sizeof(struct path_root)); // kzalloc is not necessarry
    root->drive_no = drive_no;
    root->first = 0;
    return root;
}

// returns allocated string containing partent filename and sets path pointer to next filename
// e.g. path: "script/editor/syntax_highlighter" -> "editor/syntax_highlighter"; return "script"
static const char* pparser_get_path_part(const char** path)
{
    char* path_part = 0;

    if (**path == '\0' || **path == '/')
        goto out;

    path_part = kzalloc(WHITEOS_MAX_PATH);

    int i = 0;
    while (**path != '/' && **path != '\0')
        path_part[i++] = *(*path)++;

    if(**path == '/')
        (*path)++;
out:
    return path_part;
}

// creates struct path_part for given char* path, and updates parent to point to it.
struct path_part* pparser_parse_path_part(struct path_part* parent, const char** path)
{
    struct path_part* path_part_next = 0;
    const char* path_part_str = pparser_get_path_part(path);
    if(path_part_str == 0)
        goto out;
    path_part_next = kmalloc(sizeof(struct path_part)); // kzalloc is not necessarry
    path_part_next->name = path_part_str;

    if(parent) // points to parent folder, not drive. will be 0 when parsing root->first
        parent->next = path_part_next;
out:
    return path_part_next;
}

// free the path list from heap.
void pparser_free(struct path_root* root)
{
    struct path_part* path_part = root->first;
    while (path_part)
    {
        struct path_part* path_part_next = path_part->next;
        kfree((void*)path_part->name);
        kfree((void*)path_part);
        path_part = path_part_next;
    }
    kfree(root);
}

// public function for other funcions to call and get path list from string
struct path_root* pparser_parse(const char* path, const char* curr_path)
{
    const char* tmp_path = path;
    struct path_root* root = 0;

    // return 0 if too long path string
    if (strlen(tmp_path) > WHITEOS_MAX_PATH)
        goto out;

    int disk_no = pparser_get_drive_from_path(&tmp_path);

    // return 0 if invalid disk name
    if (disk_no < 0)
        goto out;


    root = pparser_create_root(disk_no);

    // assign first path_part to root
    struct path_part* part = pparser_parse_path_part(0, &tmp_path);

    if(!part)
        goto out;

    root->first = part;

    while (tmp_path[0])
        part = pparser_parse_path_part(part, &tmp_path);

out:
    return root;
}

