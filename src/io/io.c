#include "io.h"

#define UNUSED_PORT 0x80

void iowait()
{
    outb(UNUSED_PORT, 0);
}
