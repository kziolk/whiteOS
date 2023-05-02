#ifndef PPARSER_H
#define PPARSER_H

struct path_part;

struct path_root
{
    int drive_no;
    struct path_part* first;
};

struct path_part
{
    const char* name;
    struct path_part* next;
};

struct path_root* pparser_parse(const char* path, const char* curr_path);
void pparser_free(struct path_root* root);


#endif