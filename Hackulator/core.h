#pragma once

// value defines
#define OK 0
#define ERROR -1
#define U64 unsigned long long
#define EXITCODE_ERROR -1
#define MAX_LEN_FILEPATHS 256

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
    static void StringFilter(char* pString, const char* pCharacters);
    static char* StringFindFirstCharacter(char* pSource, const char* const pCharacters);
    static char* StringFindLastCharacter(char* const pSource, const char* const pCharacters);
    static bool CharContains(char Char, const char* pCharacters);
    static bool CheckStringFormat(const char* pNumber, E_NUMBERFORMAT Format);
    static int NumToString(U64 Number, E_NUMBERFORMAT Format, char* pResult, size_t LenResult);
    static U64 PowULL(U64 Base, int Exponent);
    static void Exit(int ExitCode);
};