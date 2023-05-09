#pragma once
#include <stddef.h>
#include <stdint.h>

size_t strlen(const char* str);
size_t strnlen(const char* str, size_t limit);
size_t strnlen_terminator(const char* str, size_t limit, char terminator);

void toString(int number, char* strBuff);
void toStringHex(uint32_t number, char* strBfuff);
void toStringFloat(float number, char* strBuff, int precission);
int toInt(const char* str);

int isdigit(const char c);
char tolower(char c);

void strcpy(char* dest, const char* src);

int strncmp(const char* str1, const char* str2, int n);
int istrncmp(const char* str1, const char* str2, int n);
