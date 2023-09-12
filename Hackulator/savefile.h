#pragma once

#include "core.h"

#define CSAVEFILE_NUMSHORTNAME_BINARY       "b"
#define CSAVEFILE_NUMSHORTNAME_DUAL         "u"
#define CSAVEFILE_NUMSHORTNAME_OCTAL        "o"
#define CSAVEFILE_NUMSHORTNAME_DECIMAL      "d"
#define CSAVEFILE_NUMSHORTNAME_HEXADECIMAL  "x"

// default values
// operator labels
#define CSAVEFILE_DEFAULT_OPLABEL_ADD           "+"
#define CSAVEFILE_DEFAULT_OPLABEL_SUBTRACT      "-"
#define CSAVEFILE_DEFAULT_OPLABEL_MULTIPLY      "*"
#define CSAVEFILE_DEFAULT_OPLABEL_DIVIDE        "/"
#define CSAVEFILE_DEFAULT_OPLABEL_EXPONENTIAL   "\""
#define CSAVEFILE_DEFAULT_OPLABEL_MODULO        "%"
#define CSAVEFILE_DEFAULT_OPLABEL_AND           "&"
#define CSAVEFILE_DEFAULT_OPLABEL_OR            "|"
#define CSAVEFILE_DEFAULT_OPLABEL_XOR           "^"
#define CSAVEFILE_DEFAULT_OPLABEL_INVERT        "~"
#define CSAVEFILE_DEFAULT_OPLABEL_REVERT        "<"
#define CSAVEFILE_DEFAULT_OPLABEL_LSHIFT        "<<"
#define CSAVEFILE_DEFAULT_OPLABEL_RSHIFT        ">>"
#define CSAVEFILE_DEFAULT_OPLABEL_BRACKET_OPEN  "("
#define CSAVEFILE_DEFAULT_OPLABEL_BRACKET_CLOSE ")"

// save file defines
#define CSAVEFILE_LINE_LEN 16

class CMainLogic;

class CSaveFile
{
public:
    enum E_SAVEKEYS
    {
        // ##### ADD ENUM ONLY DIRECTLY ABOVE 'AMOUNT_KEYS' #####
        SK_NUMPREFIX_BINARY,
        SK_NUMPREFIX_DUAL,
        SK_NUMPREFIX_OCTAL,
        SK_NUMPREFIX_DECIMAL,
        SK_NUMPREFIX_HEXADECIMAL,
        SK_OPPREFIX_ADD,
        SK_OPPREFIX_SUBTRACT,
        SK_OPPREFIX_MULTIPLY,
        SK_OPPREFIX_DIVIDE,
        SK_OPPREFIX_EXPONENTIAL,
        SK_OPPREFIX_MODULO,
        SK_OPPREFIX_AND,
        SK_OPPREFIX_OR,
        SK_OPPREFIX_XOR,
        SK_OPPREFIX_INVERT,
        SK_OPPREFIX_REVERT,
        SK_OPPREFIX_LSHIFT,
        SK_OPPREFIX_RSHIFT,
        SK_OPPREFIX_BRACKET_OPEN,
        SK_OPPREFIX_BRACKET_CLOSE,
        SK_RESULTORDER,
        SK_STARTFULLSCREEN,
        AMOUNT_KEYS,
        // ######################################################
    };

    typedef struct
    {
        E_SAVEKEYS m_Key;
        char aDefault[CSAVEFILE_LINE_LEN];
    }S_SAVEKEYDEFAULT;

    CSaveFile(CMainLogic *pMainLogic, char *pSaveFilePath);
    ~CSaveFile();
    int CreateSaveFile();
    int WriteKey(E_SAVEKEYS Key, const char* pValue);
    int ReadKey(E_SAVEKEYS Key, char *pKey);

private:
    CMainLogic *m_pMainLogic;
    char m_aSaveFilePath[MAX_LEN_FILEPATHS];
    S_SAVEKEYDEFAULT m_asSaveKeyDefaults[AMOUNT_KEYS] =
    {
        { SK_NUMPREFIX_BINARY,          "0" CSAVEFILE_NUMSHORTNAME_BINARY },
        { SK_NUMPREFIX_DUAL,            "0" CSAVEFILE_NUMSHORTNAME_DUAL },
        { SK_NUMPREFIX_OCTAL,           "0" CSAVEFILE_NUMSHORTNAME_OCTAL },
        { SK_NUMPREFIX_DECIMAL,         "0" CSAVEFILE_NUMSHORTNAME_DECIMAL },
        { SK_NUMPREFIX_HEXADECIMAL,     "0" CSAVEFILE_NUMSHORTNAME_HEXADECIMAL },
        { SK_OPPREFIX_ADD,              CSAVEFILE_DEFAULT_OPLABEL_ADD },
        { SK_OPPREFIX_SUBTRACT,         CSAVEFILE_DEFAULT_OPLABEL_SUBTRACT },
        { SK_OPPREFIX_MULTIPLY,         CSAVEFILE_DEFAULT_OPLABEL_MULTIPLY },
        { SK_OPPREFIX_DIVIDE,           CSAVEFILE_DEFAULT_OPLABEL_DIVIDE },
        { SK_OPPREFIX_EXPONENTIAL,      CSAVEFILE_DEFAULT_OPLABEL_EXPONENTIAL },
        { SK_OPPREFIX_MODULO,           CSAVEFILE_DEFAULT_OPLABEL_MODULO },
        { SK_OPPREFIX_AND,              CSAVEFILE_DEFAULT_OPLABEL_AND },
        { SK_OPPREFIX_OR,               CSAVEFILE_DEFAULT_OPLABEL_OR },
        { SK_OPPREFIX_XOR,              CSAVEFILE_DEFAULT_OPLABEL_XOR },
        { SK_OPPREFIX_INVERT,           CSAVEFILE_DEFAULT_OPLABEL_INVERT },
        { SK_OPPREFIX_REVERT,           CSAVEFILE_DEFAULT_OPLABEL_REVERT },
        { SK_OPPREFIX_LSHIFT,           CSAVEFILE_DEFAULT_OPLABEL_LSHIFT },
        { SK_OPPREFIX_RSHIFT,           CSAVEFILE_DEFAULT_OPLABEL_RSHIFT },
        { SK_OPPREFIX_BRACKET_OPEN,     CSAVEFILE_DEFAULT_OPLABEL_BRACKET_OPEN },
        { SK_OPPREFIX_BRACKET_CLOSE,    CSAVEFILE_DEFAULT_OPLABEL_BRACKET_CLOSE },
        { SK_RESULTORDER,               CSAVEFILE_NUMSHORTNAME_BINARY CSAVEFILE_NUMSHORTNAME_DUAL CSAVEFILE_NUMSHORTNAME_OCTAL CSAVEFILE_NUMSHORTNAME_DECIMAL CSAVEFILE_NUMSHORTNAME_HEXADECIMAL },
        { SK_STARTFULLSCREEN,           "0" },
    };
};