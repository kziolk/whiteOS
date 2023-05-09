#pragma once

#include <stddef.h>

void* memset(void* ptr, int c, size_t size);
int memcmp(void* ptr1, void* ptr2, size_t size);
void memcpy(void* dest, void* src, size_t len);

