#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "savefile.h"
#include "mainlogic.h"
#include "core.h"

CSaveFile::CSaveFile(CMainLogic *pMainLogic, char *pSaveFilePath)
{
	int retval = 0;
	bool fileExists = false;
	char aCurrentVersion[CSAVEFILE_LINE_LEN] = { 0 };

    m_pMainLogic = pMainLogic;
    memset(m_aSaveFilePath, 0, ARRAYSIZE(m_aSaveFilePath));
    strncpy(m_aSaveFilePath, pSaveFilePath, ARRAYSIZE(m_aSaveFilePath));

	// check if save file exists
	fileExists = CCore::CheckFileExists(m_aSaveFilePath);
	
	// does not exist, create it
	if (!fileExists)
	{
		retval = CreateSaveFile();
		if (retval != OK)
		{
			CCore::Exit(EXITCODE_ERR_SAVEFILE);
		}
	}
	else// save file exists, load
	{
		// load save file
		// ...
	}
}

CSaveFile::~CSaveFile()
{

}

int CSaveFile::CreateSaveFile()
{
	FILE* pFile = 0;
	int retval = 0;

	// create save file
	pFile = fopen(m_aSaveFilePath, "w");
	if (!pFile)
	{
		m_pMainLogic->m_Log.LogErr("Creating save file '%s'", m_aSaveFilePath);
		return ERROR;
	}

	// write default values
	for (int i = 0; i < ARRAYSIZE(m_asSaveKeyDefaults); ++i)
	{
		retval = WriteKey(m_asSaveKeyDefaults[i].m_Key, m_asSaveKeyDefaults[i].aDefault);
		if (retval != OK)
		{
			m_pMainLogic->m_Log.LogErr("Writing default key %d", m_asSaveKeyDefaults[i].m_Key);
			fclose(pFile);
			return ERROR;
		}
	}

	fclose(pFile);
	return OK;
}

int CSaveFile::WriteKey(E_SAVEKEYS Key, const char* pValue)
{
	FILE* pFile = 0;
	int line = 0;
	char aLine[AMOUNT_KEYS][CSAVEFILE_LINE_LEN] = { { 0 } };
	signed char ch = 0;
	int chCount = 0;
	int lineIndex = 0;
	int bufIndex = 0;
	int eofReached = false;

	// read
	pFile = fopen(m_aSaveFilePath, "r");
	if (!pFile)
	{
		m_pMainLogic->m_Log.LogErr("Opening save file for reading");
		return ERROR;
	}

	// read all lines
	while (1)
	{
		ch = getc(pFile);

		// safety
		if (chCount >= ARRAYSIZE(aLine) * ARRAYSIZE(aLine[0]) || lineIndex >= ARRAYSIZE(aLine))
		{
			break;
		}

		if (ch == EOF)
		{
			ch = '\0';
			eofReached = true;
		}

		aLine[lineIndex][bufIndex] = ch;
		bufIndex++;

		if (ch == '\n' || eofReached)
		{
			bufIndex = 0;
			lineIndex++;
		}

		if (eofReached)
			break;

		chCount++;
	}

	fclose(pFile);
	line = Key;

	// change value
	snprintf(aLine[line], ARRAYSIZE(aLine[0]), "%s\n", pValue);

	// write back
	pFile = fopen(m_aSaveFilePath, "w");
	if (!pFile)
	{
		m_pMainLogic->m_Log.LogErr("Opening file for writing");
		return ERROR;
	}

	for (int i = 0; i < ARRAYSIZE(aLine); ++i)
		fwrite(aLine[i], sizeof(char), strlen(aLine[i]), pFile);

	fclose(pFile);

	return OK;
}

int CSaveFile::ReadKey(E_SAVEKEYS Key, char* pKey)
{
	FILE* pFile = 0;
	int line = 0;
	char aLine[AMOUNT_KEYS][CSAVEFILE_LINE_LEN] = { 0 };
	signed char ch = 0;
	int chCount = 0;
	int lineIndex = 0;
	int bufIndex = 0;
	int eofReached = false;

	// read
	pFile = fopen(m_aSaveFilePath, "r");
	if (!pFile)
	{
		m_pMainLogic->m_Log.LogErr("Opening file for reading");
		return ERROR;
	}

	// read all lines
	while (1)
	{
		ch = getc(pFile);

		// safety
		if (chCount >= ARRAYSIZE(aLine) * ARRAYSIZE(aLine[0]))
		{
			break;
		}

		if (ch == EOF)
		{
			ch = '\0';
			eofReached = true;
		}

		aLine[lineIndex][bufIndex] = ch;
		bufIndex++;

		if (ch == '\n' || eofReached)
		{
			if (lineIndex == Key)
			{
				memset(pKey, 0, CSAVEFILE_LINE_LEN);
				CCore::StringCopyIgnore(pKey, aLine[lineIndex], CSAVEFILE_LINE_LEN, "\n");
				fclose(pFile);
				return OK;
			}

			bufIndex = 0;
			lineIndex++;
		}

		if (eofReached)
			break;

		chCount++;
	}

	fclose(pFile);

	m_pMainLogic->m_Log.LogErr("Reading line %d", Key);

	return ERROR;
}