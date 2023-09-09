#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "core.h"

CCore::CCore()
{

}

CCore::~CCore()
{

}

int CCore::StringCompareNocase(const char* pSource, const char* pDest, size_t Len)
{
	signed char diff = 0;

	for (int i = 0; i < Len; ++i)
	{
		if (!pSource[i] && !pDest[i])
			break;

		diff = tolower(pSource[i]) - tolower(pDest[i]);

		if (diff != 0)
			return diff;
	}

	return 0;
}

void CCore::StringRevert(char* pString)
{
	int i = 0;
	const int stringLen = strlen(pString);
	char* aTemp = (char*)malloc(stringLen);

	if (aTemp == NULL)
		return;

	memset(aTemp, 0, stringLen);
	memcpy(aTemp, pString, stringLen);

	for (i = 0; i < stringLen; ++i)
		pString[i] = aTemp[stringLen - 1 - i];

	free(aTemp);
}

const char* CCore::StringFindFirstCharacter(const char* pSource, const char* pCharacters)
{
	for (; *pSource; ++pSource)
	{
		for (; *pCharacters; ++pCharacters)
		{
			if (*pSource == *pCharacters)
				return pCharacters;
		}
	}

	return NULL;
}

bool CCore::CharContains(char Char, const char* pCharacters)
{
	int i = 0;

	for (; *pCharacters; ++pCharacters)
	{
		if (Char == *pCharacters)
			return true;
	}

	return false;
}

bool CCore::CheckStringFormat(const char* pNumber, E_NUMBERFORMAT Format)
{
	switch (Format)
	{
	case NUMFORM_BINARY:
		for (; *pNumber; ++pNumber)
		{
			if (*pNumber < '0' || *pNumber > '1')
				return false;
		}
		break;

	case NUMFORM_DUAL:
		for (; *pNumber; ++pNumber)
		{
			if (*pNumber < '0' || *pNumber > '3')
				return false;
		}
		break;

	case NUMFORM_OCTAL:
		for (; *pNumber; ++pNumber)
		{
			if (*pNumber < '0' || *pNumber > '7')
				return false;
		}
		break;

	case NUMFORM_DECIMAL:
		for (; *pNumber; ++pNumber)
		{
			if (*pNumber < '0' || *pNumber > '9')
				return false;
		}
		break;

	case NUMFORM_HEXADECIMAL:
		for (; *pNumber; ++pNumber)
		{
			if (!((*pNumber >= '0' && *pNumber <= '9') ||
				(*pNumber >= 'A' && *pNumber <= 'F') ||
				(*pNumber >= 'a' && *pNumber <= 'f')))
				return false;
		}
		break;
	}

	return true;
}

int CCore::NumToString(U64 Number, E_NUMBERFORMAT Format, char* pResult, size_t LenResult)
{
	int i = 0;
	int mask = 0;
	int rshift = 0;
	int digit = 0;
	char aDigit[2] = { 0 };
	int lastPosRelevant = 0;

	switch (Format)
	{
	case NUMFORM_BINARY:
		mask = 0x01;
		rshift = 1;
		break;

	case NUMFORM_DUAL:
		mask = 0x03;
		rshift = 2;
		break;

	case NUMFORM_OCTAL:
		mask = 0x07;
		rshift = 3;
		break;

	case NUMFORM_DECIMAL:
		snprintf(pResult, LenResult, "%llu", Number);
		return OK;

	case NUMFORM_HEXADECIMAL:
		snprintf(pResult, LenResult, "%llX", Number);
		return OK;

	default:
		return ERROR;
		break;
	}

	for (i = 0; i < MIN(sizeof(U64) * 8, LenResult); i++)
	{
		digit = Number & mask;
		snprintf(aDigit, ARRAYSIZE(aDigit), "%d", digit);
		Number >>= rshift;

		if (aDigit[0] != '0')
			lastPosRelevant = i;

		pResult[i] = aDigit[0];
	}

	pResult[lastPosRelevant + 1] = '\0';

	StringRevert(pResult);

	return OK;
}