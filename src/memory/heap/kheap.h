#pragma once 
#include <stddef.h>

void kheap_init();
void* kmalloc(size_t size);
void* kzalloc(size_t size);
