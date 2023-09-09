#pragma once

#define TRUE (1 == 1)
#define FALSE (!TRUE)

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

void revertString(char *pString);
void filterString(char *pInput, char *pCharacters);
int strncmp_nocase(char *pStr1, char *pStr2, int Size);
unsigned long long powull(unsigned long long Base, int Exponent);
int strIsDecimalNumber(char *pNumber);
int strIsBinaryNumber(char *pNumber);
int strIsHexNumber(char *pNumber);
void swapUll(unsigned long long *xp, unsigned long long *yp);