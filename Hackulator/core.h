#pragma once

// value defines
#define OK 0
#define ERROR -1
#define U64 unsigned long long

// function defines
#define ARRAYSIZE(a) (sizeof(a) / sizeof(a[0]))
#define STRINGIFY_NAME(a) #a
#define STRINGIFY_VALUE(a) STRINGIFY_NAME(a)
#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

class CCore
{
public:
    enum E_NUMBERFORMAT
    {
        NUMFORM_BINARY,
        NUMFORM_DUAL,
        NUMFORM_OCTAL,
        NUMFORM_DECIMAL,
        NUMFORM_HEXADECIMAL,
    };

    CCore();
    ~CCore();
    static int StringCompareNocase(const char* pSource, const char* pDest, size_t Len);
    static void StringRevert(char *pString);
    static const char* StringFindFirstCharacter(const char* pSource, const char* pCharacters);
    static bool CharContains(char Char, const char* pCharacters);
    static bool CheckStringFormat(const char* pNumber, E_NUMBERFORMAT Format);
    static int NumToString(U64 Number, E_NUMBERFORMAT Format, char* pResult, size_t LenResult);
};