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
	else// save file exists, evolve and load
	{
		// evolve save file if necessary
		retval = EvolveSaveFile();
		if (retval != OK)
		{
			CCore::Exit(EXITCODE_ERR_SAVEFILE);
		}

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
		if (retval != ERROR)
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
		fprintf(pFile, aLine[i]);

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

// Hope we never need this
int CSaveFile::EvolveSaveFile()
{
	int retval = 0;
	int currentVersion = 0;
	int latestVersion = 0;
	char aCurrentVersion[CSAVEFILE_LINE_LEN] = { 0 };

	// check save file version
	retval = ReadKey(SK_SAVEFILE_VERSION, aCurrentVersion);
	if (retval != OK)
	{
		m_pMainLogic->m_Log.LogErr("Reading save file version");
		CCore::Exit(EXITCODE_ERR_SAVEFILE);
	}

	// versions differ
	if (strncmp(CSAVEFILE_SAVEFILE_VERSION, aCurrentVersion, ARRAYSIZE(aCurrentVersion)) == 0)
		return OK;
	else
		m_pMainLogic->m_Log.Log("Save file versions differ (current/new): %s/%s\nInitiating save file evolution...", aCurrentVersion, CSAVEFILE_SAVEFILE_VERSION);

	currentVersion= atoi(aCurrentVersion);
	latestVersion = atoi(CSAVEFILE_SAVEFILE_VERSION);

	// check if versions differentiate
	if (currentVersion == latestVersion)
		return OK;

	// go through all necessary evolutionary steps to reach the current save file format
	for (int i = currentVersion; i < latestVersion; ++i)
	{
		m_pMainLogic->m_Log.Log("%d --> %d", i, i + 1);

		if (i == 0)
		{
			// ...
		}
	}

	m_pMainLogic->m_Log.Log("Evolution from version %d to %d completed successfully, you can now proceed as usual!", currentVersion, latestVersion);

	return OK;
}