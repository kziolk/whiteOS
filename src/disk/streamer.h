#pragma once

#include "disk.h"

struct disk_stream
{
    int pos;
    struct disk* disk;
};

// higher level API for automating reading from disk sector by sector

struct disk_stream* diskstreamer_new(int disk_id);
int diskstreamer_seek(struct disk_stream* stream, int pos);
int diskstreamer_read(struct disk_stream* stream, void* out, int total);
void diskstreamer_close(struct disk_stream* stream);


