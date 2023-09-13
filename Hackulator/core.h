#pragma once

// value defines
#define OK 0
#define ERROR -1
#define U64 unsigned long long
#define MAX_LEN_FILEPATHS 256

// exit codes
#define EXITCODE_OK 0
#define EXITCODE_ERR_SAVEFILE -1

// function defines
#define ARRAYSIZE(a) (sizeof(a) / sizeof(a[0]))
#define STRINGIFY_NAME(a) #a
#define STRINGIFY_VALUE(a) STRINGIFY_NAME(a)
#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

class CCore
{
public:
    CCore();
    ~CCore();
    static int StringCompareNocase(const char* pSource, const char* pDest, size_t Len);
    static void StringCopyIgnore(char* pDest, const char* pSource, size_t Len, const char* pIgnore);
    static void StringRevert(char *pString);
    static void StringFilter(char* pString, const char* pCharacters);
    static char* StringFindFirstCharacter(char* pSource, const char* const pCharacters);
    static char* StringFindLastCharacter(char* const pSource, const char* const pCharacters);
    static bool CharContains(char Char, const char* pCharacters);
    static bool CheckFileExists(const char* pFilePath);
    static U64 PowULL(U64 Base, int Exponent);
    static void Exit(int ExitCode);
};