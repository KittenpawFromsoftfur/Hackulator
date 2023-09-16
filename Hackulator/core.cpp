#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#include "core.h"

CCore::CCore()
{

}

CCore::~CCore()
{

}

int CCore::StringCompareNocase(const char* pSource, const char* pCompare, size_t Len)
{
	signed char diff = 0;

	for (int i = 0; i < Len; ++i)
	{
		if (!pSource[i] && !pCompare[i])
			break;

		diff = tolower(pSource[i]) - tolower(pCompare[i]);

		if (diff != 0)
			return diff;
	}

	return 0;
}

void CCore::StringCopyIgnore(char* pDest, const char* pSource, size_t Len, const char* pIgnore)
{
	int destIndex = 0;
	int ignLen = strlen(pIgnore);
	bool ignore = false;

	for (int i = 0; i < Len; ++i)
	{
		if (!pSource[i])
			break;

		// check if next source char should be ignored
		ignore = false;

		for (int ign = 0; ign < ignLen; ++ign)
		{
			if (pSource[i] == pIgnore[ign])
			{
				ignore = true;
				break;
			}
		}

		if (ignore)
		{
			destIndex++;
			continue;
		}

		pDest[destIndex] = pSource[i];
		destIndex++;
	}
}

void CCore::StringRevert(char* pString)
{
	int i = 0;
	const size_t stringLen = strlen(pString);
	char* aTemp = (char*)malloc(stringLen);

	if (aTemp == NULL)
		return;

	memset(aTemp, 0, stringLen);
	memcpy(aTemp, pString, stringLen);

	for (i = 0; i < stringLen; ++i)
		pString[i] = aTemp[stringLen - 1 - i];

	free(aTemp);
}

void CCore::StringFilter(char *pString, const char *pCharacters)
{
	size_t i = 0;
	size_t ii = 0;
	const size_t strlenCharacters = strlen(pCharacters);
	bool foundOccurrence = false;
	int pos = 0;

	for (i = 0; i < strlen(pString); ++i)
	{
		foundOccurrence = false;

		for (ii = 0; ii < strlenCharacters; ++ii)
		{
			if (pString[i] == pCharacters[ii])
			{
				foundOccurrence = true;
				break;
			}
		}

		if (!foundOccurrence)
		{
			pString[pos] = pString[i];
			pos++;
		}
	}

	pString[pos] = '\0';
}

char* CCore::StringFindFirstCharacter(char* pSource, const char* const pCharacters)
{
	const char* pCharactersTemp = 0;

	for (; *pSource; ++pSource)
	{
		for (pCharactersTemp = pCharacters; *pCharactersTemp; ++pCharactersTemp)
		{
			if (*pSource == *pCharactersTemp)
				return pSource;
		}
	}

	return NULL;
}

char* CCore::StringFindLastCharacter(char* const pSource, const char* const pCharacters)
{
	const size_t strlenSource = strlen(pSource);
	char* pSourceEnd = pSource + strlenSource * sizeof(char) - 1;
	const char* pCharactersTemp = 0;

	for (; *pSourceEnd; --pSourceEnd)
	{
		for (pCharactersTemp = pCharacters; *pCharactersTemp; ++pCharactersTemp)
		{
			if (*pSourceEnd == *pCharactersTemp)
				return pSourceEnd;
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

bool CCore::CheckFileExists(const char* pFilePath)
{
	FILE* pFile = 0;

	pFile = fopen(pFilePath, "r");
	if (!pFile)
	{
		return false;
	}
	else
	{
		fclose(pFile);
		return true;
	}
}

U64 CCore::PowULL(U64 Base, int Exponent)
{
	int i = 0;
	U64 value = 1;

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

int GetFlags(int Value, int Flags)
{
	return (Value | Flags);
}

void CCore::Exit(int ExitCode)
{
	system("pause");
	exit(ExitCode);
}