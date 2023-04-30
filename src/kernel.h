#pragma once
#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 20

void kernel_main(void* multiboot_info_ptr, uint32_t magic_number);
void terminal_writestring(const char* str);


