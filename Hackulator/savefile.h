#pragma once

#include "core.h"

class CMainLogic;

class CSaveFile
{
public:
    CSaveFile(CMainLogic *pMainLogic, char *pSaveFilePath);
    ~CSaveFile();
    int Init();

private:
    CMainLogic *m_pMainLogic;
    char m_aSaveFilePath[MAX_LEN_FILEPATHS];
};