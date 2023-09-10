#include <stdio.h>
#include <string.h>

#include "savefile.h"
#include "mainlogic.h"
#include "core.h"

CSaveFile::CSaveFile(CMainLogic *pMainLogic, char *pSaveFilePath)
{
    m_pMainLogic = pMainLogic;
    memset(m_aSaveFilePath, 0, ARRAYSIZE(m_aSaveFilePath));

    strncpy(m_aSaveFilePath, pSaveFilePath, ARRAYSIZE(m_aSaveFilePath));
}

CSaveFile::~CSaveFile()
{

}

int CSaveFile::Init()
{
    FILE* pFile = 0;

    pFile = fopen(m_aSaveFilePath, "w");

    if (!pFile)
    {
        m_pMainLogic->m_Log.Log("Failed to open savefile '%s'", m_aSaveFilePath);
        return ERROR;
    }

    return OK;
}