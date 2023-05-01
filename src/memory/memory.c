#include "memory.h"

void* memset(void* ptr, int c, size_t size)
{
    char* c_ptr = (char*) ptr;
    for (int i = 0; i < size; i++)
    {
        c_ptr[i] = (char) c;
    }

    return ptr;
}

int memcmp(void* ptr1, void* ptr2, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        if(*(char*)(ptr1++) != *(char*)(ptr2++))
            return ((char*)ptr1)[-1] < ((char*)ptr2)[-1] ? -1 : 1;
    }
    return 0;
}