#pragma once

#include "core.h"
#include "mainlogic.h"

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

class CSaveFile
{
public:
    enum E_SAVEKEYS
    {
        // ##### ADD ENUM ONLY DIRECTLY ABOVE 'AMOUNT_SAVEKEYS' #####
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
        SK_RESPREFIXVIS,
        SK_STARTFULLSCREEN,
        SK_AUTOSAVE,
        SK_INPUTFORMAT,
        AMOUNT_SAVEKEYS,
        // ######################################################
    };

    typedef struct
    {
        E_SAVEKEYS m_Key;
        char m_aValue[CSAVEFILE_LINE_LEN];
        char m_aDefaultValue[CSAVEFILE_LINE_LEN];
    }S_SAVEKEY;

    CSaveFile(CMainLogic *pMainLogic, char *pSaveFilePath);
    ~CSaveFile();
    int LoadSaveFile();
    int SaveSaveFile();
    int ResetSaveFile();

    S_SAVEKEY m_asSaveKeys[AMOUNT_SAVEKEYS] =
    {
        { SK_NUMPREFIX_BINARY,          "", "0" CMAINLOGIC_NUMSHORTNAME_BINARY },
        { SK_NUMPREFIX_DUAL,            "", "0" CMAINLOGIC_NUMSHORTNAME_DUAL },
        { SK_NUMPREFIX_OCTAL,           "", "0" CMAINLOGIC_NUMSHORTNAME_OCTAL },
        { SK_NUMPREFIX_DECIMAL,         "", "0" CMAINLOGIC_NUMSHORTNAME_DECIMAL },
        { SK_NUMPREFIX_HEXADECIMAL,     "", "0" CMAINLOGIC_NUMSHORTNAME_HEXADECIMAL },
        { SK_OPPREFIX_ADD,              "", CSAVEFILE_DEFAULT_OPLABEL_ADD },
        { SK_OPPREFIX_SUBTRACT,         "", CSAVEFILE_DEFAULT_OPLABEL_SUBTRACT },
        { SK_OPPREFIX_MULTIPLY,         "", CSAVEFILE_DEFAULT_OPLABEL_MULTIPLY },
        { SK_OPPREFIX_DIVIDE,           "", CSAVEFILE_DEFAULT_OPLABEL_DIVIDE },
        { SK_OPPREFIX_EXPONENTIAL,      "", CSAVEFILE_DEFAULT_OPLABEL_EXPONENTIAL },
        { SK_OPPREFIX_MODULO,           "", CSAVEFILE_DEFAULT_OPLABEL_MODULO },
        { SK_OPPREFIX_AND,              "", CSAVEFILE_DEFAULT_OPLABEL_AND },
        { SK_OPPREFIX_OR,               "", CSAVEFILE_DEFAULT_OPLABEL_OR },
        { SK_OPPREFIX_XOR,              "", CSAVEFILE_DEFAULT_OPLABEL_XOR },
        { SK_OPPREFIX_INVERT,           "", CSAVEFILE_DEFAULT_OPLABEL_INVERT },
        { SK_OPPREFIX_REVERT,           "", CSAVEFILE_DEFAULT_OPLABEL_REVERT },
        { SK_OPPREFIX_LSHIFT,           "", CSAVEFILE_DEFAULT_OPLABEL_LSHIFT },
        { SK_OPPREFIX_RSHIFT,           "", CSAVEFILE_DEFAULT_OPLABEL_RSHIFT },
        { SK_OPPREFIX_BRACKET_OPEN,     "", CSAVEFILE_DEFAULT_OPLABEL_BRACKET_OPEN },
        { SK_OPPREFIX_BRACKET_CLOSE,    "", CSAVEFILE_DEFAULT_OPLABEL_BRACKET_CLOSE },
        { SK_RESULTORDER,               "", CMAINLOGIC_NUMSHORTNAME_BINARY CMAINLOGIC_NUMSHORTNAME_DUAL CMAINLOGIC_NUMSHORTNAME_OCTAL CMAINLOGIC_NUMSHORTNAME_DECIMAL CMAINLOGIC_NUMSHORTNAME_HEXADECIMAL },
        { SK_RESPREFIXVIS,              "", CMAINLOGIC_NUMSHORTNAME_BINARY CMAINLOGIC_NUMSHORTNAME_DUAL CMAINLOGIC_NUMSHORTNAME_OCTAL CMAINLOGIC_NUMSHORTNAME_DECIMAL CMAINLOGIC_NUMSHORTNAME_HEXADECIMAL },
        { SK_STARTFULLSCREEN,           "", "0" },
        { SK_AUTOSAVE,                  "", "1" },
        { SK_INPUTFORMAT,               "", "" },// needs to be set in constructor because enum E_NUMTYPES is not preprocessor capable
    };

private:
    int CreateSaveFile();
    int WriteKey(E_SAVEKEYS Key, const char* pValue);
    int ReadKey(E_SAVEKEYS Key, char* pValue);

    CMainLogic *m_pMainLogic;
    char m_aSaveFilePath[MAX_LEN_FILEPATHS];
};