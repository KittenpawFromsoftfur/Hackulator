#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "functions.h"

/*********************************************************************************
* @brief Reverts a string sequence
*
* @param[out] *pString Input string
*********************************************************************************/
void revertString(char *pString)
{
    int i = 0;
    const int stringLen = strlen(pString);
    char *aTemp = (char*)malloc(stringLen);

    if (aTemp == NULL)
        return;

    memset(aTemp, 0, stringLen);
    memcpy(aTemp, pString, stringLen);

    for (i = 0; i < stringLen; ++i)
        pString[i] = aTemp[stringLen - 1 - i];

    free(aTemp);
}

/*********************************************************************************
* @brief Filters characters out of a string
*
* @param[out] *pString     Input string
* @param[in]  *pCharacters Characters to be filtered
*********************************************************************************/
void filterString(char *pInput, char *pCharacters)
{
    int i = 0;
    int ii = 0;
    int strlenCharacters = strlen(pCharacters);
    int foundOccurrence = FALSE;
    int pos = 0;

    for (i = 0; i < strlen(pInput); ++i)
    {
        foundOccurrence = FALSE;

        for (ii = 0; ii < strlenCharacters; ++ii)
        {
            if (pInput[i] == pCharacters[ii])
            {
                foundOccurrence = TRUE;
                break;
            }
        }

        if (!foundOccurrence)
        {
            pInput[pos] = pInput[i];
            pos++;
        }
    }

    pInput[pos] = '\0';
}

/*********************************************************************************
* @brief Compares two strings case insensitively
*
* @param[in] *pStr1 String 1
* @param[in] *pStr2 String 2
* @param[in] Size   Max amount of characters to compare
*
* @retval 0  The strings match
* @retval <0 pStr1 is less than pStr2
* @retval >0 pStr1 is more than pStr2
*********************************************************************************/
int strncmp_nocase(char *pStr1, char *pStr2, int Size)
{
    int cnt = 0;
    int d = 0;

    for (;; pStr1++, pStr2++)
    {
        if (cnt >= Size)
            break;

        d = tolower((unsigned char)*pStr1) - tolower((unsigned char)*pStr2);

        if (d != 0 || !*pStr1)
            return d;

        cnt++;
    }

    return 0;
}

/*********************************************************************************
* @brief Calculates the power of a number
*
* @param[in] Base     Base
* @param[in] Exponent Exponent
*
* @retval Result
*********************************************************************************/
unsigned long long powull(unsigned long long Base, int Exponent)
{
    int i = 0;
    unsigned long long value = 1;

    for (i = 0; i < sizeof(int) * 8; ++i)
    {
        if (Exponent & (1 << i))
        {
            value *= Base;
        }

        Base *= Base;
    }

    return value;
}

/*********************************************************************************
* @brief Checks whether a string is in the decimal number format
*
* @param[in] *pNumber String
*
* @retval TRUE
* @retval FALSE
*********************************************************************************/
int strIsDecimalNumber(char *pNumber)
{
    for (; *pNumber; ++pNumber)
    {
        if (*pNumber < '0' || *pNumber > '9')
            return FALSE;
    }

    return TRUE;
}

/*********************************************************************************
* @brief Checks whether a string is in the binary number format
*
* @param[in] *pNumber String
*
* @retval TRUE
* @retval FALSE
*********************************************************************************/
int strIsBinaryNumber(char *pNumber)
{
    for (; *pNumber; ++pNumber)
    {
        if (*pNumber < '0' || *pNumber > '1')
            return FALSE;
    }

    return TRUE;
}

/*********************************************************************************
* @brief Checks whether a string is in the hex number format
*
* @param[in] *pNumber String
*
* @retval TRUE
* @retval FALSE
*********************************************************************************/
int strIsHexNumber(char *pNumber)
{
    for (; *pNumber; ++pNumber)
    {
        if (!((*pNumber >= '0' && *pNumber <= '9') ||
            (*pNumber >= 'A' && *pNumber <= 'F') ||
            (*pNumber >= 'a' && *pNumber <= 'f')))
            return FALSE;
    }

    return TRUE;
}

/*********************************************************************************
* @brief Swaps two numbers
*
* @param[in] *xp First number
* @param[in] *yp Second number
*********************************************************************************/
void swapUll(unsigned long long *xp, unsigned long long *yp)
{
    unsigned long long temp = *xp;
    *xp = *yp;
    *yp = temp;
}