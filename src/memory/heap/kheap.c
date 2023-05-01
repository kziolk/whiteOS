#include "kheap.h"
#include "heap.h"
#include "memory/memory.h"
#include "terminal/terminal.h"
#include "whitelib/string.h"
struct heap kernel_heap;
struct heap_table kernel_heap_table;

static char numstr_buff[32];
// kernel heap is 100MB
void kheap_init() {
    int total_table_entries = WHITEOS_HEAP_SIZE_BYTES / WHITEOS_HEAP_BLOCK_SIZE;
    kernel_heap_table.entries = (HEAP_BLOCK_TABLE_ENTRY*)(WHITEOS_HEAP_TABLE_ADDRESS);
    kernel_heap_table.total = total_table_entries;

    void* end = (void*)(WHITEOS_HEAP_ADDRESS + WHITEOS_HEAP_SIZE_BYTES);
    int res = heap_create(&kernel_heap, (void*)(WHITEOS_HEAP_ADDRESS), end, &kernel_heap_table);
    if (res < 0) {
        // todo kernel panic
        terminal_print("create result: ");
        toString(res, numstr_buff);
        terminal_print(numstr_buff);

        terminal_print("\ntotal table entries: ");
        toString(total_table_entries, numstr_buff);
        terminal_print(numstr_buff);

        terminal_print("\nAddr start: ");
        toStringHex(WHITEOS_HEAP_ADDRESS, numstr_buff);
        terminal_print(numstr_buff);
        terminal_print("\nAddr end: ");
        toStringHex((int)end, numstr_buff);
        terminal_print(numstr_buff);
        
    }
}

void* kmalloc(size_t size) {
    return heap_malloc(&kernel_heap, size);    
}

void* kzalloc(size_t size) {
    void* ptr = kmalloc(size);
    if (!ptr) 
        return 0;
    memset(ptr, 0x00, size);
    return ptr;
}

void kfree(void* ptr) {
    heap_free(&kernel_heap, ptr);
}
