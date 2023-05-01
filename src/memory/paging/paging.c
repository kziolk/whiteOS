#include "paging.h"
#include "memory/heap/kheap.h"
#include "status.h"
#include <stdint.h>

void paging_load_directory(uint32_t* directory);
static uint32_t* current_directory = 0;

struct paging_4gb_chunk* paging_new_4gb(uint8_t flags) {
    uint32_t* directory = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
    int offset = 0;
    for (int i = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; i++) {
        uint32_t* table = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
        for (int e = 0; e < PAGING_TOTAL_ENTRIES_PER_TABLE; e++) {
            table[e] = (offset + (e * PAGING_PAGE_SIZE)) | flags;
        }
        offset += PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE;
        directory[i] = (uint32_t)table | flags | PAGING_IS_WRITEABLE;
    }
    // 4B variable allocated on stack taking 4KiB
    struct paging_4gb_chunk* chunk_4gb = kzalloc(sizeof(struct paging_4gb_chunk));
    chunk_4gb->directory_entry = directory;
    return chunk_4gb;
}

void paging_switch(uint32_t* directory) {
    paging_load_directory(directory);
    current_directory = directory;
}

uint32_t* paging_4gb_chunk_get_directory(struct paging_4gb_chunk* chunk) {
    return chunk->directory_entry;
}

int paging_is_aligned(void* addr) {
    return (uint32_t)addr % PAGING_PAGE_SIZE == 0;
}

int paging_get_indexes(void* virtual_address, uint32_t* directory_index_out, uint32_t* table_index_out) {
    int res = 0;
    if (!paging_is_aligned(virtual_address)) {
        res = -EINVARG;
        goto out;
    }
    *directory_index_out = ((uint32_t)virtual_address / (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE));
    *table_index_out = ((uint32_t)virtual_address % (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE)) / PAGING_PAGE_SIZE;
out:
    return res;
}

int paging_set(uint32_t* directory, void* virtual_address, uint32_t val) {
    int res = 0;
    if (!paging_is_aligned(virtual_address)) {
        res = -EINVARG;
        goto out;
    }
    uint32_t directory_index, table_index;
    res = paging_get_indexes(virtual_address, &directory_index, &table_index);
    if (res < 0)
        goto out;
    uint32_t entry = directory[directory_index];
    uint32_t* table = (uint32_t*)( entry & 0xfffff000 );
    table[table_index] = val;
out:
    return res;
}
