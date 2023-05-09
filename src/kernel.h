#pragma once

#include <stdint.h>

void kernel_main(void* multiboot_info_ptr, uint32_t magic_number);
void panic(const char* msg);

#define ERROR(value) (void*)(value)
#define ERROR_I(value) (int)(value)
#define ISERR(value) ((int)value < 0)
