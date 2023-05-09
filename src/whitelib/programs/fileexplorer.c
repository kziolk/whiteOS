#include "fileexplorer.h"
#include "fs/file.h"
#include "terminal/terminal.h"
#include "keyboard/keyboard.h"
#include "memory/memory.h"
#include "whitelib/string.h"

void print_contents(const char* path, int depth)
{
    for (int i = 0; i < depth; i++)
        terminal_writechar('\t');
    terminal_print(path);
    terminal_writechar('\n');
    int root_file = fopen(path, "dir");
    struct dirent dent;
    while (readdir(root_file, &dent) == 0)
    {
        char subpath[128];
        memset(subpath, 0, 128);
        strcpy(subpath, path);
        if (strlen(path) != 3)
            subpath[strlen(subpath)] = '/';
        
        strcpy(&subpath[strlen(subpath)], dent.name);
        if (strncmp(dent.name, ".", 1) != 0 && strncmp(dent.name, "..", 2) != 0)
            print_contents(subpath, depth+1);
    }
}

void list_path(const char* path)
{
    terminal_print("List path ");
    terminal_print(path);
    terminal_writechar('\n');

    int root_file = fopen(path, "dir");
    struct dirent dent;
    while (readdir(root_file, &dent) == 0)
    {
        terminal_writechar('\t');
        terminal_print(dent.name);
        terminal_writechar('\n');
    }
}

void fileexplorer()
{
    terminal_clear();
    terminal_print("File explorer\n");
    print_contents("0:/", 0);
    terminal_print("\nPress any key to exit\n");
    keyboard_scan_key();
    keyboard_scan_key();
    return;
}
