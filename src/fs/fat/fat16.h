#pragma once

#include "file.h"

struct filesystem* fat16_init();

struct fat_directory_item;
int fat16_isdir(struct fat_directory_item* item);
void fat16_get_full_relative_filename(struct fat_directory_item* item, char* out, int max_len);