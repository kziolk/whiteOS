#pragma once
#include <stddef.h>
#include <stdint.h>

size_t strlen(const char* str);

void toString(int number, char* strBuff);
void toStringHex(uint32_t number, char* strBfuff);
void toStringFloat(double number, char* strBuff, int precission);

int isdigit(const char c);
