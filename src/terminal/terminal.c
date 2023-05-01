#include "terminal.h"
#include "whitelib/string.h"

#include <stdint.h>
#include <stddef.h>

static uint16_t* video_mem = (uint16_t*)(0xb8000);
static int row = 0;
static int col = 0;
static char colour = 7;

static void move_cursor(int offset);

void terminal_gotoxy(int x, int y) {
    col = x;
    row = y;
}

void terminal_getxy(int* x, int* y) {
    *x = col;
    *y = row;
}

void terminal_putchar(char c) {
    video_mem[col + (row * VGA_WIDTH)] = (colour << 8) | c;
}

void terminal_writechar(char c) {
    if (c == '\n') {
        row = (row + 1) % VGA_HEIGHT;
        col = 0;
        return;
    }
    if (c == '\t')
    {
        col = (col / 4) * 4 + 4;
        if (col >= VGA_WIDTH)
        {
            col = 0;
            row = (row + 1) % VGA_HEIGHT;
        }
        return;
    }
    video_mem[(row * VGA_WIDTH) + col] = (colour << 8) | c;
    col = (col + 1) % VGA_WIDTH;
    if (!col) {
        row = (row + 1) % VGA_HEIGHT;
    }
}

void terminal_print(const char* str) {
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++) {
        terminal_writechar(str[i]);
    }
}

void terminal_clear() {
    for (int x = 0; x < VGA_WIDTH; x++)
        for (int y = 0; y < VGA_HEIGHT; y++)
            video_mem[x + (y * VGA_WIDTH)] = (colour << 8) | ' ';
    row = col = 0;
}
