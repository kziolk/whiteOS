#include "string.h"
#include <stddef.h>

size_t strnlen(const char* str, size_t limit) {
    size_t i = 0;
    while(str[i] != '\0') {
        if (i == limit) return 0;
        i++;
    }
    return i;
}
size_t strlen(const char* str) {
    size_t i = 0;
    while(str[i] != 0)
        i++;
    return i;
}
size_t strnlen_terminator(const char* str, size_t limit, char terminator)
{
    size_t i = 0;
    while(str[i] != '\0' || str[i] != terminator) {
        if (i == limit) return 0;
        i++;
    }
    return i;
}

void toString(int number, char* strBuff) {
    if (number == 0) {
        strBuff[0] = '0';
        strBuff[1] = 0;
        return;
    }
    int left = 0;
    if (number < 0) {
        number *= -1;
        strBuff[0] = '-';
        left = 1;
    }
    char tmpBuff[120];
    int digits = 0;
    while (number != 0) {
        tmpBuff[digits] = '0' + (number % 10);
        number /= 10;
        digits++;
    }
    int end = left + digits;
    for (; left < end; left++) {
        digits--;
        strBuff[left] = tmpBuff[digits];
    }
    strBuff[end] = 0;
}

void toStringHex(uint32_t number, char* strBuff) {
    strBuff[0] = '0';
    strBuff[1] = 'x';
    strBuff[10] = 0;

    for (int i = 9; i > 1; i--) {
        strBuff[i] = '0' * (number % 16 < 10) + ('A' - 10) * (number % 16 >= 10) + number % 16;
        number /= 16;
    }
}

void toStringFloat(float number, char* strBuff, int precission) {
    toString((int)number, strBuff);
    if (precission < 1) return;

    size_t coma_id = strlen(strBuff);
    strBuff[coma_id] = '.';

    if (number < 0) number *= -1;
    number = (number - (int)number);
    for (int i = 0; i <= precission ; i++) {
        number *= 10;
    }
    if ((int)number % 10 > 4)
        number += 10;

    toString((int)(number / 10), strBuff + coma_id + 1);
}

int isdigit(const char c) {
    return c <= '9' && c >= '0';
}

char tolower(char c)
{
    if (c >= 'A' && c <= 'Z')
        return c - 'A' + 'a';
    return c;
}

int toInt(const char* str)
{
    size_t i = 0;
    int val = 0;
    while (str[i]) {
        val *= 10;
        val += (str[i]-'0');
        i++;
    }
    return val;
}

int strncmp(const char* str1, const char* str2, int n)
{
    for (int i = 0; i < n; i++)
        if (str1[i] != str2[i])
            return str1[i] - str2[i];
        else if (str1[i] == '\0')
            break;
    return 0;
}

int istrncmp(const char* str1, const char* str2, int n)
{
    for (int i = 0; i < n; i++)
        if (str1[i] != str2[i] && tolower(str1[i]) != tolower(str2[i]))
            return str1[i] - str2[i];
        else if (str1[i] == '\0')
            break;
    return 0;
}

void strcpy(char* dest, const char* src)
{
    while (*src) {
    *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
}

