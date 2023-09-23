#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "savefile.h"
#include "mainlogic.h"
#include "core.h"

CSaveFile::CSaveFile(CMainLogic *pMainLogic, char *pSaveFilePath)
{
    m_pMainLogic = pMainLogic;
	m_IsInitialized = false;
    memset(m_aSaveFilePath, 0, ARRAYSIZE(m_aSaveFilePath));
    strncpy(m_aSaveFilePath, pSaveFilePath, ARRAYSIZE(m_aSaveFilePath));
}

CSaveFile::~CSaveFile()
{

}

int CSaveFile::SetDefaultKey(E_SAVEKEYS Key, const char *pString)
{
	if (Key < 0 || Key >= AMOUNT_SAVEKEYS)
	{
		m_pMainLogic->m_Log.LogErr("Key out of range: %d", Key);
		return ERROR;
	}

	strncpy(m_asSaveKeys[Key].m_aDefaultValue, pString, ARRAYSIZE(m_asSaveKeys[0].m_aDefaultValue));
	return OK;
}

int CSaveFile::Init()
{
	int retval = 0;
	bool fileExists = false;

	// check if save file exists
	fileExists = CCore::CheckFileExists(m_aSaveFilePath);

	// if it does not exist, create it
	if (!fileExists)
	{
		retval = CreateSaveFile();
		if (retval != OK)
			return ERROR;
	}

	m_IsInitialized = true;
	return OK;
}

int CSaveFile::LoadSaveFile()
{
	int retval = 0;
	
	// check if initialized
	if (!m_IsInitialized)
	{
		m_pMainLogic->m_Log.LogErr("Save file is not initialized");
		return ERROR;
	}

	for (int i = 0; i < AMOUNT_SAVEKEYS; ++i)
	{
		retval = ReadKey((E_SAVEKEYS)i, m_asSaveKeys[i].m_aValue);
		if (retval != OK)
		{
			m_pMainLogic->m_Log.LogErr("Loading save file, reading key %d", i);
			return ERROR;
		}

		strncpy(m_asSaveKeys[i].m_aValue, m_asSaveKeys[i].m_aValue, ARRAYSIZE(m_asSaveKeys[0].m_aValue));
	}

	return OK;
}

int CSaveFile::SaveSaveFile()
{
	int retval = 0;

	// check if initialized
	if (!m_IsInitialized)
	{
		m_pMainLogic->m_Log.LogErr("Save file is not initialized");
		return ERROR;
	}

	for (int i = 0; i < AMOUNT_SAVEKEYS; ++i)
	{
		retval = WriteKey((E_SAVEKEYS)i, m_asSaveKeys[i].m_aValue);
		if (retval != OK)
		{
			m_pMainLogic->m_Log.LogErr("Saving save file, writing key %d", i);
			return ERROR;
		}

		strncpy(m_asSaveKeys[i].m_aValue, m_asSaveKeys[i].m_aValue, ARRAYSIZE(m_asSaveKeys[0].m_aValue));
	}

	return OK;
}

int CSaveFile::ResetSaveFile()
{
	// check if initialized
	if (!m_IsInitialized)
	{
		m_pMainLogic->m_Log.LogErr("Save file is not initialized");
		return ERROR;
	}

	return CreateSaveFile();
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
	for (int i = 0; i < ARRAYSIZE(m_asSaveKeys); ++i)
	{
		retval = WriteKey(m_asSaveKeys[i].m_Key, m_asSaveKeys[i].m_aDefaultValue);
		if (retval != OK)
		{
			m_pMainLogic->m_Log.LogErr("Writing default key %d", m_asSaveKeys[i].m_Key);
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
	char aLine[AMOUNT_SAVEKEYS][CSAVEFILE_LINE_LEN] = { { 0 } };
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

int CSaveFile::ReadKey(E_SAVEKEYS Key, char* pValue)
{
	FILE* pFile = 0;
	int line = 0;
	char aLine[AMOUNT_SAVEKEYS][CSAVEFILE_LINE_LEN] = { 0 };
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
				memset(pValue, 0, CSAVEFILE_LINE_LEN);
				CCore::StringCopyIgnore(pValue, aLine[lineIndex], CSAVEFILE_LINE_LEN, "\n");
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