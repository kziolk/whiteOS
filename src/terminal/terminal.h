#pragma once 

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

void terminal_gotoxy(int x, int y);
void terminal_getxy(int* x, int* y);

void terminal_putchar(char c);
void terminal_writechar(char c);
void terminal_print(const char* str);
void terminal_clear();
