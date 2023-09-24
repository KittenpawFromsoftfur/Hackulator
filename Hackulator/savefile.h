#pragma once

#include "core.h"

// save file defines
#define CSAVEFILE_LINE_LEN 16

class CMainLogic;

class CSaveFile
{
public:
    enum E_SAVEKEYS
    {
        // ##### ADD ENUM ONLY DIRECTLY ABOVE 'AMOUNT_SAVEKEYS' #####
        SK_INPPREFIX_BINARY,
        SK_INPPREFIX_DUAL,
        SK_INPPREFIX_OCTAL,
        SK_INPPREFIX_DECIMAL,
        SK_INPPREFIX_HEXADECIMAL,
        SK_INPPREFIX_ASCII,
        SK_OPPREFIX_ADD,
        SK_OPPREFIX_SUBTRACT,
        SK_OPPREFIX_MULTIPLY,
        SK_OPPREFIX_DIVIDE,
        SK_OPPREFIX_EXPONENTIAL,
        SK_OPPREFIX_MODULO,
        SK_OPPREFIX_AND,
        SK_OPPREFIX_OR,
        SK_OPPREFIX_XOR,
        SK_OPPREFIX_LSHIFT,
        SK_OPPREFIX_RSHIFT,
        SK_OPPREFIX_INVERT,
        SK_OPPREFIX_REVERT,
        SK_OPPREFIX_BRACKET_OPEN,
        SK_OPPREFIX_BRACKET_CLOSE,
        SK_RESULTORDER,
        SK_RESPREFIXVIS,
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
    int SetDefaultKey(E_SAVEKEYS Key, const char* pString);
    int Init();
    int LoadSaveFile();
    int SaveSaveFile();
    int ResetSaveFile();

    S_SAVEKEY m_asSaveKeys[AMOUNT_SAVEKEYS] =
    {
        { SK_INPPREFIX_BINARY,          "", "" },
        { SK_INPPREFIX_DUAL,            "", "" },
        { SK_INPPREFIX_OCTAL,           "", "" },
        { SK_INPPREFIX_DECIMAL,         "", "" },
        { SK_INPPREFIX_HEXADECIMAL,     "", "" },
        { SK_INPPREFIX_ASCII,           "", "" },
        { SK_OPPREFIX_ADD,              "", "" },
        { SK_OPPREFIX_SUBTRACT,         "", "" },
        { SK_OPPREFIX_MULTIPLY,         "", "" },
        { SK_OPPREFIX_DIVIDE,           "", "" },
        { SK_OPPREFIX_EXPONENTIAL,      "", "" },
        { SK_OPPREFIX_MODULO,           "", "" },
        { SK_OPPREFIX_AND,              "", "" },
        { SK_OPPREFIX_OR,               "", "" },
        { SK_OPPREFIX_XOR,              "", "" },
        { SK_OPPREFIX_LSHIFT,           "", "" },
        { SK_OPPREFIX_RSHIFT,           "", "" },
        { SK_OPPREFIX_INVERT,           "", "" },
        { SK_OPPREFIX_REVERT,           "", "" },
        { SK_OPPREFIX_BRACKET_OPEN,     "", "" },
        { SK_OPPREFIX_BRACKET_CLOSE,    "", "" },
        { SK_RESULTORDER,               "", "" },
        { SK_RESPREFIXVIS,              "", "" },
        { SK_AUTOSAVE,                  "", "" },
        { SK_INPUTFORMAT,               "", "" },
    };

private:
    int CreateSaveFile();
    int WriteKey(E_SAVEKEYS Key, const char* pValue);
    int ReadKey(E_SAVEKEYS Key, char* pValue);

    CMainLogic *m_pMainLogic;
    char m_aSaveFilePath[MAX_LEN_FILEPATHS];
    bool m_IsInitialized;
};