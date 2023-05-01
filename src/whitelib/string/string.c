#include "string.h"

size_t strlen(const char* str) {
    size_t i = 0;
    while(str[i] != 0) 
        i++;
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

void toStringFloat(double number, char* strBuff, int precission) {
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
