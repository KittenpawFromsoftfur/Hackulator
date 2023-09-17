#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <Windows.h>
#include <stdexcept>
#include <stdarg.h>

#include "mainlogic.h"
#include "savefile.h"
#include "core.h"
#include "log.h"

CMainLogic::CMainLogic(bool StartFullscreen, char *pSaveFilePath)
{
	int retval = 0;

	// default initialize members
	m_ExitApplication = false;
	m_DefaultNumberType = NUT_DECIMAL;
	m_AutoSave = false;
	memset(m_aResultOrder, 0, ARRAYSIZE(m_aResultOrder));
	memset(m_aResultPrefixVis, 0, ARRAYSIZE(m_aResultPrefixVis));

	m_StartFullscreen = StartFullscreen;
	if (m_StartFullscreen)
		ShowWindow(GetConsoleWindow(), SW_MAXIMIZE);

	m_pSaveFile = new CSaveFile(this, pSaveFilePath);

	// load save data (overwrites some members)
	retval = LoadSaveData();
	if (retval != OK)
		CCore::Exit(EXITCODE_ERR_MAINLOGIC);
}

CMainLogic::~CMainLogic()
{
	delete m_pSaveFile;
}

int CMainLogic::EntryPoint()
{
	int retval = 0;
	char* pToken = 0;
	char* pRest = 0;
	char aInput[CMAINLOGIC_CONSOLE_BUFFERLEN] = { 0 };
	S_INPUTTOKENS sInputTokens = { 0 };
	int tokenIndex = 0;

	// parse input loop
	while (1)
	{
		// check for application exit request
		if (m_ExitApplication)
			return EXITCODE_OK;
	
		// resets
		pToken = 0;
		pRest = aInput;
		memset(aInput, 0, ARRAYSIZE(aInput));
		memset(&sInputTokens, 0, sizeof(sInputTokens));
		tokenIndex = 0;

		// get user input
		retval = scanf(" %" STRINGIFY_VALUE(CMAINLOGIC_CONSOLE_BUFFERLEN) "[^\n]", aInput);

		if (retval <= 0)
		{
			m_Log.LogErr("Failed to scan input");
			continue;
		}

		// parse input
		retval = ParseInput(aInput, ARRAYSIZE(aInput), &sInputTokens);
		if (retval != OK)
			continue;

		// evaluate tokens
		EvaluateTokens(&sInputTokens);
	}

    return EXITCODE_OK;
}

void CMainLogic::RequestApplicationExit()
{
	m_ExitApplication = true;
}

int CMainLogic::OnConsoleWindowStateChange()// laststop: get this to work
{
	DWORD modeFlags = 0;
	int retval = 0;

	retval = GetConsoleDisplayMode(&modeFlags);
	if (!retval)
	{
		m_Log.LogErr("Getting display mode");
		return ERROR;
	}

	m_Log.Log("modeflags %d", modeFlags);

	// save
	retval = SaveSaveData();
	if (retval != OK)
		return ERROR;

	return OK;
}

int CMainLogic::ParseInput(const char* pInput, size_t LenInput, S_INPUTTOKENS *psInputTokens)
{
	char aTokenBuf[ARRAYSIZE(psInputTokens->m_aInputTokens[0])] = { 0 };
	char aTempBuf[ARRAYSIZE(aTokenBuf)] = { 0 };
	bool stringEnd = false;
	bool charIsDelim = false;
	bool charIsOperator = false;
	bool copyToken = false;
	int inputTokenIndex = 0;

	for (int i = 0; i < LenInput; ++i)
	{
		copyToken = false;

		if (!pInput[i] || i >= LenInput - 1)
			stringEnd = true;

		// determine if char is operator
		charIsOperator = false;

		for (int opInd = 0; opInd < ARRAYSIZE(m_asOperators); ++opInd)
		{
			if (CCore::CharContains(pInput[i], m_asOperators[opInd].m_aOperator))
			{
				charIsOperator = true;
				break;
			}
		}

		// determine if char is delimiter
		if (pInput[i] == ' ' || pInput[i] == '\t')
			charIsDelim = true;
		else
			charIsDelim = false;

		// copy char for char to current token buffer
		// if char is delimiter, it will not be copied to buffer, instead the previous token will be copied if it is non-empty
		if (charIsDelim)
		{
			// only if token buffer is non-empty, it will be copied
			if (strnlen(aTokenBuf, ARRAYSIZE(aTokenBuf)) > 0)
				copyToken = true;
		}
		else if (charIsOperator)// if char is operator
		{
			// it will only be copied to token buffer if the buffer is empty
			if (strnlen(aTokenBuf, ARRAYSIZE(aTokenBuf)) == 0)
			{
				snprintf(aTempBuf, ARRAYSIZE(aTempBuf), "%s%c", aTokenBuf, pInput[i]);
				strncpy(aTokenBuf, aTempBuf, ARRAYSIZE(aTokenBuf));
				copyToken = true;
			}
			else// otherwise the token will be finished and the char pointer set back by one
			{
				i--;
				copyToken = true;
			}
		}
		else// if char is normal char, it will be copied to token buffer
		{
			snprintf(aTempBuf, ARRAYSIZE(aTempBuf), "%s%c", aTokenBuf, pInput[i]);
			strncpy(aTokenBuf, aTempBuf, ARRAYSIZE(aTokenBuf));
		}

		// copy token, also if string has reached the end and token buffer is filled
		if (copyToken || (stringEnd && strnlen(aTokenBuf, ARRAYSIZE(aTokenBuf)) > 0))
		{
			// check if max tokens exceeded
			if (inputTokenIndex >= ARRAYSIZE(psInputTokens->m_aInputTokens[0]))
			{
				m_Log.LogErr("Max. tokens exceeded, max. %d", ARRAYSIZE(psInputTokens->m_aInputTokens[0]));
				return ERROR;
			}

			// copy token buffer to current token
			strncpy(psInputTokens->m_aInputTokens[inputTokenIndex], aTokenBuf, ARRAYSIZE(psInputTokens->m_aInputTokens[0]));

			// null buffers
			memset(aTempBuf, 0, ARRAYSIZE(aTempBuf));
			memset(aTokenBuf, 0, ARRAYSIZE(aTokenBuf));

			// prepare next token
			inputTokenIndex++;
		}

		if (stringEnd)
			break;
	}

	return OK;
}

int CMainLogic::EvaluateTokens(S_INPUTTOKENS *psInputTokens)
{
	int retval = 0;
	S_TOKEN asToken[ARRAYSIZE(psInputTokens->m_aInputTokens)];
	int amountTokens = 0;
	U64 result = 0;

	memset(asToken, 0, ARRAYSIZE(asToken) * sizeof(S_TOKEN));

	// look for command
	for (int i = 0; i < ARRAYSIZE(m_asCommands); ++i)
	{
		// if a command was entered, execute command and leave
		if (CCore::StringCompareNocase(psInputTokens->m_aInputTokens[0], m_asCommands[i].m_aName, ARRAYSIZE(psInputTokens->m_aInputTokens[0])) == 0)
			return ExecuteCommand(&m_asCommands[i], psInputTokens);
	}
	
	// parse tokens
	for (int i = 0; i < ARRAYSIZE(psInputTokens->m_aInputTokens); ++i)
	{
		if (strnlen(psInputTokens->m_aInputTokens[i], ARRAYSIZE(psInputTokens->m_aInputTokens[0])) == 0)
			break;

		// determine token type
		// check for numbers
		if (ExtractNumberFromToken(psInputTokens->m_aInputTokens[i], &asToken[i].m_Number) == OK)
		{
			asToken[i].m_TokType = TOT_NUMBER;
		}// check for operators
		else if ((asToken[i].m_OpType = GetOperatorFromToken(psInputTokens->m_aInputTokens[i])->m_OpType) != (E_OPTYPES)-1)
		{
			asToken[i].m_TokType = TOT_OPERATOR;
		}
		else
		{
			asToken[i].m_TokType = (E_TOKTYPES)-1;
		}

		// check token validity and fill rest of token data
		if (asToken[i].m_TokType == (E_TOKTYPES)-1)
		{
			m_Log.LogErr("%d. token '%s' is an invalid token (expected %s number format)", i + 1, psInputTokens->m_aInputTokens[i], GetNumberFromType(m_DefaultNumberType)->m_aName);
			return ERROR;
		}

		strncpy(asToken[i].m_aToken, psInputTokens->m_aInputTokens[i], ARRAYSIZE(psInputTokens->m_aInputTokens[0]));
		amountTokens++;

		//if (asToken[i].m_TokType == TOT_NUMBER)
		//	m_Log.Log("--> %llu", asToken[i].number);
		//else
		//	m_Log.Log("--> %s", asToken[i].aToken);
	}

	// evaluate syntax
	retval = CheckSyntax(asToken, amountTokens);
	if (retval != OK)
		return ERROR;

	// calculate tokens
	result = Calculate(asToken, amountTokens);

	// print result
	PrintResult(result);
	
	return OK;
}

int CMainLogic::ExecuteCommand(S_COMMAND *psCommand, S_INPUTTOKENS *psInputTokens)
{
	int retval = 0;
	E_COMRETVALS retvalCom = CRV_OK;
	bool printHelpOnError = true;
	bool doAutoSave = false;
	bool forceAutoSave = false;

	switch (psCommand->m_ID)
	{
	case COM_HELP:
		retvalCom = ComHelp((E_COMMANDS) - 1);
		break;

	case COM_SET_INPUTFORMAT:
		retvalCom = ComSetinputformat(psInputTokens->m_aInputTokens[1]);
		doAutoSave = true;
		break;

	case COM_SET_NUMBERPREFIX:
		retvalCom = ComSetnumberprefix(psInputTokens->m_aInputTokens[1], psInputTokens->m_aInputTokens[2]);
		doAutoSave = true;
		break;

	case COM_SET_OPERATOR:
		retvalCom = ComSetoperator(psInputTokens->m_aInputTokens[1], psInputTokens->m_aInputTokens[2]);
		doAutoSave = true;
		break;

	case COM_SET_RESULTORDER:
		retvalCom = ComSetresultorder(psInputTokens->m_aInputTokens[1]);
		doAutoSave = true;
		break;

	case COM_SET_RESPREFIXVIS:
		retvalCom = ComSetresultprefixvisibility(psInputTokens->m_aInputTokens[1]);
		doAutoSave = true;
		break;

	case COM_SET_AUTOSAVE:
		retvalCom = ComSetautosave(psInputTokens->m_aInputTokens[1]);
		doAutoSave = true;
		forceAutoSave = true;// because if you disable it, that would not be saved
		break;

	case COM_SAVE:
		retvalCom = ComSave(psInputTokens->m_aInputTokens[1]);
		break;

	case COM_RESETSETTINGS:
		retvalCom = ComResetSettings(psInputTokens->m_aInputTokens[1]);
		break;

	case COM_CLEARSCREEN:
		retvalCom = ComClearscreen();
		break;

	case COM_EXIT:
		retvalCom = ComExit();
		break;
	}

	// failure handling
	if (retvalCom != CRV_OK)
	{
		switch (retvalCom)
		{
		case CRV_ERROR:
			// own error message is already output by function
			break;

		case CRV_INVALIDPARAMS:
			m_Log.LogErr("Invalid parameters");
			break;

		case CRV_CANCELLED:
			m_Log.Log("Operation cancelled");
			printHelpOnError = false;
			break;
		}

		if (printHelpOnError)
			ComHelp(psCommand->m_ID);
	}

	// saving
	if (doAutoSave && (m_AutoSave || forceAutoSave))
	{
		retval = SaveSaveData();
		if (retval != OK)
			return ERROR;
	}

	if (retvalCom != CRV_OK)
		return ERROR;

	return OK;
}

int CMainLogic::ExtractNumberFromToken(const char* paToken, U64 *pNumber)
{
	int retval = 0;
	E_NUMTYPES numType = NUT_INVALID;
	char aPrefix[CMAINLOGIC_CONSOLE_NUMBER_PREFIX_LENGTH + 1] = { 0 };
	char aContent[CMAINLOGIC_CONSOLE_TOKEN_SIZE] = { 0 };
	bool hasPrefix = true;
	const char* pConvertableInput = aContent;

	if (!pNumber)
		return ERROR;
	
	// check the number type
	// copy prefix
	strncpy(aPrefix, paToken, ARRAYSIZE(aPrefix));
	aPrefix[ARRAYSIZE(aPrefix) -1] = '\0';

	// copy content
	strncpy(aContent, paToken + CMAINLOGIC_CONSOLE_NUMBER_PREFIX_LENGTH, ARRAYSIZE(aContent));

	numType = NUT_INVALID;

	numType = GetNumberFromPrefix(aPrefix)->m_NumType;

	// no prefix given
	if (numType == NUT_INVALID)
	{
		hasPrefix = false;
		pConvertableInput = paToken;// if no prefix is given, the convertable input is the token instead of the content which is missing some characters

		// check if non-prefixed input is matching the format of the default type
		numType = GetNumberFromType(m_DefaultNumberType)->m_NumType;
	}

	// type still cannot be determined, return
	if (numType == NUT_INVALID)
		return ERROR;

	// check the number format (only necessary if the input is prefixed, since non-prefixed input has already been checked... see above)
	if (hasPrefix)
	{
		if (!CheckStringFormat(pConvertableInput, m_asNumbers[numType].m_NumType))
			return ERROR;
	}

	// convert input to number
	*pNumber = strtoull(pConvertableInput, NULL, m_asNumbers[numType].m_Radix);

	return OK;
}

int CMainLogic::CheckSyntax(S_TOKEN* pasToken, size_t AmountTokens)
{
	return OK;

	/*
	int prevOpFlags = 0;
	S_TOKEN *psPrevious = 0;
	S_TOKEN* psCurrent = 0;
	U64 synflags = 0;

	// check token for token
	for (int i = 0; i < AmountTokens; ++i)
	{
		// resets
		psCurrent = &pasToken[i];
		
		if (i > 0)
			psPrevious = &pasToken[i - 1];
		
		// check tokens who are operators
		if (psCurrent->m_TokType == TOT_OPERATOR)
		{
			// first operator must not be combining
			// however some combining operators double as number-modifying, keep that in mind
			if (!psPrevious)
			{
				if ((GetOperatorFlags(psCurrent->m_OpType, OPFLAG_COMBINE) > 0) &&
					(GetOperatorFlags(psCurrent->m_OpType, OPFLAG_MODIFYNUM) == 0))
				{
					m_Log.LogErr("%d. token '%s', first token must not be a combining operator", i + 1, psCurrent->m_aToken);
					return ERROR;
				}
			}
			else// 2nd+ operators
			{
				// if previous operator was combining or modifies a number, the next token has to be a number
				if ((GetOperatorFlags(psCurrent->m_OpType, prevOpFlags) & (OPFLAG_COMBINE || OPFLAG_MODIFYNUM)) > 0)
				{
					m_Log.LogErr("%d. token '%s', number-combining or number-modifying operator must be followed by a number", i + 1, psCurrent->m_aToken);
					return ERROR;
				}

				// last token must not be an operator
				if (i >= AmountTokens - 1)
				{
					m_Log.LogErr("%d. token '%s', last token must not be a number-combining or number-modifying operator", i + 1, psCurrent->m_aToken);
					return ERROR;
				}
			}

			// prepare next loop
			prevOpFlags = GetOperatorFlags(psCurrent->m_OpType, OPFLAG_ALL);
		}
		else if (psCurrent->m_TokType == TOT_NUMBER)// check tokens who are numbers
		{
			// first number
			if (!psPrevious)
			{
				// everything allowed
			}
			else// 2nd+ number
			{
				// if previous token is a number, error
				if (psPrevious->m_TokType == TOT_NUMBER)
				{
					m_Log.LogErr("%d. token '%s', number must not be followed by another number", i + 1, psCurrent->m_aToken);
					return ERROR;
				}
			}

			// prepare next loop
			prevOpFlags = 0;
		}

		// prepare next loop
		psPrevious = psCurrent;
	}

	return OK;*/
}

U64 CMainLogic::Calculate(S_TOKEN* pasToken, size_t AmountTokens)
{
	S_TOKEN asCalculation[CMAINLOGIC_CONSOLE_TOKENS];
	S_TOKEN asTempCalculation[ARRAYSIZE(asCalculation)];
	S_TOKEN sTempToken;
	bool anyCalculationsLeft = true;
	U64 result = 0;
	int firstNumIndex = 0;
	int firstOperatorIndex = 0;
	int secondNumIndex = 0;
	int secondOperatorIndex = 0;
	int amountCurrentTokens = AmountTokens;
	U64 tempResult = 0;
	int amountTokensDisposed = 0;

	// reset structs
	memset(asCalculation, 0, ARRAYSIZE(asCalculation) * sizeof(S_TOKEN));

	// copy original tokens
	for (int i = 0; i < AmountTokens; ++i)
		asCalculation[i] = pasToken[i];

	// calculation loop, simple from left to right
	while (1)
	{
		// repeat calculation until only one number is left
		for (int i = 0; i < amountCurrentTokens; ++i)
		{
			// reset values
			memset(asTempCalculation, 0, ARRAYSIZE(asTempCalculation) * sizeof(S_TOKEN));
			firstNumIndex = -1;
			firstOperatorIndex = -1;
			secondNumIndex = -1;
			secondOperatorIndex = -1;
			amountTokensDisposed = 0;

			// determine number and operator indices
			for (int index = 0; index < amountCurrentTokens; ++index)
			{
				// determine first num index if not yet found
				if (firstNumIndex < 0)
				{
					if (asCalculation[index].m_TokType == TOT_NUMBER)
					{
						firstNumIndex = index;

						// see if an operator is put in front
						if (index > 0)
							firstOperatorIndex = index - 1;
					}
				}
				else if (secondNumIndex < 0)// determine second num index if not yet found
				{
					if (index != firstNumIndex && asCalculation[index].m_TokType == TOT_NUMBER)
					{
						secondNumIndex = index;

						// determine second operator index
						secondOperatorIndex = index - 1;
					}
				}
				else
				{
					break;
				}
			}

			if (firstNumIndex >= 0)
				amountTokensDisposed++;

			if (firstOperatorIndex >= 0)
				amountTokensDisposed++;

			if (secondNumIndex >= 0)
				amountTokensDisposed++;

			if (secondOperatorIndex >= 0)
				amountTokensDisposed++;

			//m_Log.Log("Disposed %d, %d, %d, %d, %d", amountTokensDisposed, firstNumIndex, firstOperatorIndex, secondNumIndex, secondOperatorIndex);

			tempResult = asCalculation[firstNumIndex].m_Number;

			// calculate if two nums have been found, otherwise the result has been seen
			if (secondNumIndex < 0)
			{
				anyCalculationsLeft = false;
			}
			else// calculate
			{
				U64 firstNum = asCalculation[firstNumIndex].m_Number;
				U64 secondNum = asCalculation[secondNumIndex].m_Number;
				E_OPTYPES firstOperatorType = (E_OPTYPES)-1;
				E_OPTYPES secondOperatorType = (E_OPTYPES)-1;

				// operator types (first one does not have to have a operator)
				if (firstOperatorIndex >= 0)
					firstOperatorType = asCalculation[firstOperatorIndex].m_OpType;

				secondOperatorType = asCalculation[secondOperatorIndex].m_OpType;

				// modify numbers
				firstNum = ModifyNumberByOperator(firstNum, firstOperatorType);
				//secondNum = ModifyNumberByOperator(secondNum, secondOperatorType);

				// calculate

				switch (secondOperatorType)
				{
				case OPT_ADD:
					tempResult = firstNum + secondNum;
					break;

				case OPT_SUBTRACT:
					tempResult = firstNum - secondNum;
					break;

				case OPT_AND:
					tempResult = firstNum & secondNum;
					break;

				case OPT_OR:
					tempResult = firstNum | secondNum;
					break;

				case OPT_XOR:
					tempResult = firstNum ^ secondNum;
					break;

				case OPT_INVERT:
					tempResult = firstNum + secondNum;
					break;

				//case OPT_REVERT:
				//	tempResult = firstNum + secondNum;
				//	break;
				}

				break;
			}
		}

		// assign new token
		memset(&sTempToken, 0, sizeof(sTempToken));

		sTempToken.m_Number = tempResult;
		sTempToken.m_TokType = TOT_NUMBER;

		// crop calculation
		asTempCalculation[0] = sTempToken;

		for (int temp = 1; temp < (amountCurrentTokens - amountTokensDisposed); ++temp)
		{
			asTempCalculation[temp] = asCalculation[temp + amountTokensDisposed - 1];
		}

		memcpy(asCalculation, asTempCalculation, sizeof(asTempCalculation[0]) * ARRAYSIZE(asTempCalculation));

		amountCurrentTokens -= amountTokensDisposed;

		if (amountCurrentTokens <= 0)
			anyCalculationsLeft = false;

		// check if the calculation has finished
		if (!anyCalculationsLeft)
			break;
	}

	result = tempResult;

	return result;
}

void CMainLogic::PrintResult(U64 Result)
{
	char aNumBin[64 + 1] = { 0 };
	char aNumDua[32 + 1] = { 0 };
	char aNumOct[22 + 1] = { 0 };
	char aNumDec[20 + 1] = { 0 };
	char aNumHex[16 + 1] = { 0 };
	char* pPrefix = 0;
	char* paNum = 0;
	bool prefixVisible = false;

	NumToString(Result, NUT_BINARY, aNumBin, ARRAYSIZE(aNumBin));
	NumToString(Result, NUT_DUAL, aNumDua, ARRAYSIZE(aNumDua));
	NumToString(Result, NUT_OCTAL, aNumOct, ARRAYSIZE(aNumOct));
	NumToString(Result, NUT_DECIMAL, aNumDec, ARRAYSIZE(aNumDec));
	NumToString(Result, NUT_HEXADECIMAL, aNumHex, ARRAYSIZE(aNumHex));

	//m_Log.Log("| %s | 0x%s | 0b%s | 0d%s | 0o%s |", aNumDec, aNumHex, aNumBin, aNumDua, aNumOct);

	for (int i = 0; i < strlen(m_aResultOrder); ++i)
	{
		m_Log.LogCustom("", "", "|");

		// get prefix visibility
		prefixVisible = false;

		for (int vis = 0; vis < ARRAYSIZE(m_aResultPrefixVis); ++vis)
		{
			if (CCore::CharCompareNocase(m_aResultOrder[i], m_aResultPrefixVis[vis]) == 0)
			{
				prefixVisible = true;
				break;
			}
		}

		// get number
		switch (m_aResultOrder[i])
		{
		case CMAINLOGIC_NUMSHORTNAME_BINARY[0]:
			pPrefix = m_asNumbers[NUT_BINARY].m_aPrefix;
			paNum = aNumBin;
			break;

		case CMAINLOGIC_NUMSHORTNAME_DUAL[0]:
			pPrefix = m_asNumbers[NUT_DUAL].m_aPrefix;
			paNum = aNumDua;
			break;

		case CMAINLOGIC_NUMSHORTNAME_OCTAL[0]:
			pPrefix = m_asNumbers[NUT_OCTAL].m_aPrefix;
			paNum = aNumOct;
			break;

		case CMAINLOGIC_NUMSHORTNAME_DECIMAL[0]:
			pPrefix = m_asNumbers[NUT_DECIMAL].m_aPrefix;
			paNum = aNumDec;
			break;

		case CMAINLOGIC_NUMSHORTNAME_HEXADECIMAL[0]:
			pPrefix = m_asNumbers[NUT_HEXADECIMAL].m_aPrefix;
			paNum = aNumHex;
			break;

		default:
			m_Log.LogErr("Invalid result order at position %d '%c'", i + 1, m_aResultOrder[i]);
			return;
		}

		m_Log.LogCustom("", "", " %s%s ", prefixVisible ? pPrefix : "", paNum);
	}

	m_Log.LogCustom("", "", "|\n");
}

U64 CMainLogic::ModifyNumberByOperator(U64 Number, E_OPTYPES OpType)
{
	U64 result = Number;

	switch (OpType)
	{
	case OPT_SUBTRACT:
		result *= -1;
		break;
	}

	return result;
}

int CMainLogic::GetOperatorFlags(E_OPTYPES OpType, int OpFlags)
{
	S_OPERATOR *pOperator = 0;

	// look for matching operator in array and return flags
	for (int i = 0; i < ARRAYSIZE(m_asOperators); ++i)
	{
		if (m_asOperators[i].m_OpType == OpType)
		{
			pOperator = &m_asOperators[i];
			return (m_asOperators->m_Flags & OpFlags);
		}
	}

	m_Log.LogErr("This should not occur, operator with index %d was not define in source code", OpType);
	return 0;
}

bool CMainLogic::CheckNumberPrefixCollisions(const char* pString, S_NUMBER** ppsNumberColliding)
{
	for (int i = 0; i < ARRAYSIZE(m_asNumbers); ++i)
	{
		if (CCore::StringCompareNocase(pString, m_asNumbers[i].m_aPrefix, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
		{
			*ppsNumberColliding = &m_asNumbers[i];
			return true;
		}
	}

	return false;
}

bool CMainLogic::CheckOperatorCollisions(const char* pString, S_OPERATOR** ppsOperatorColliding)
{
	for (int i = 0; i < ARRAYSIZE(m_asOperators); ++i)
	{
		if (CCore::StringCompareNocase(pString, m_asOperators[i].m_aOperator, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
		{
			*ppsOperatorColliding = &m_asOperators[i];
			return true;
		}
	}

	return false;
}

CMainLogic::S_OPERATOR* CMainLogic::GetOperatorFromType(E_OPTYPES OpType)
{
	for (int i = 0; i < ARRAYSIZE(m_asOperators); ++i)
	{
		if (OpType == m_asOperators[i].m_OpType)
			return &m_asOperators[i];
	}

	return &m_asOperators[OPT_INVALID];
}

CMainLogic::S_OPERATOR* CMainLogic::GetOperatorFromType(const char *pType)
{
	for (int i = 0; i < ARRAYSIZE(m_asOperators); ++i)
	{
		if (CCore::StringCompareNocase(pType, m_asOperators[i].m_aName, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
			return &m_asOperators[i];
	}

	return &m_asOperators[OPT_INVALID];
}

CMainLogic::S_OPERATOR* CMainLogic::GetOperatorFromToken(const char* paToken)
{
	for (int i = 0; i < ARRAYSIZE(m_asOperators); ++i)
	{
		if (CCore::StringCompareNocase(paToken, m_asOperators[i].m_aOperator, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
			return &m_asOperators[i];
	}

	return &m_asOperators[OPT_INVALID];
}

CMainLogic::S_NUMBER* CMainLogic::GetNumberFromType(E_NUMTYPES NumType)
{
	for (int i = 0; i < ARRAYSIZE(m_asNumbers); ++i)
	{
		if (NumType == m_asNumbers[i].m_NumType)
			return &m_asNumbers[i];
	}

	return &m_asNumbers[NUT_INVALID];
}

CMainLogic::S_NUMBER* CMainLogic::GetNumberFromType(const char *pType)
{
	for (int i = 0; i < ARRAYSIZE(m_asNumbers); ++i)
	{
		if (CCore::StringCompareNocase(pType, m_asNumbers[i].m_aName, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
			return &m_asNumbers[i];
	}

	return &m_asNumbers[NUT_INVALID];
}


CMainLogic::S_NUMBER* CMainLogic::GetNumberFromPrefix(const char *pPrefix)
{
	for (int i = 0; i < ARRAYSIZE(m_asNumbers); ++i)
	{
		if (CCore::StringCompareNocase(pPrefix, m_asNumbers[i].m_aPrefix, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
			return &m_asNumbers[i];
	}

	return &m_asNumbers[NUT_INVALID];
}

bool CMainLogic::CheckStringFormat(const char* pNumber, E_NUMTYPES NumType)
{
	switch (NumType)
	{
	case NUT_BINARY:
		for (; *pNumber; ++pNumber)
		{
			if (*pNumber < '0' || *pNumber > '1')
				return false;
		}
		break;

	case NUT_DUAL:
		for (; *pNumber; ++pNumber)
		{
			if (*pNumber < '0' || *pNumber > '3')
				return false;
		}
		break;

	case NUT_OCTAL:
		for (; *pNumber; ++pNumber)
		{
			if (*pNumber < '0' || *pNumber > '7')
				return false;
		}
		break;

	case NUT_DECIMAL:
		for (; *pNumber; ++pNumber)
		{
			if (*pNumber < '0' || *pNumber > '9')
				return false;
		}
		break;

	case NUT_HEXADECIMAL:
		for (; *pNumber; ++pNumber)
		{
			if (!((*pNumber >= '0' && *pNumber <= '9') ||
				(*pNumber >= 'A' && *pNumber <= 'F') ||
				(*pNumber >= 'a' && *pNumber <= 'f')))
				return false;
		}
		break;
	}

	return true;
}

int CMainLogic::NumToString(U64 Number, E_NUMTYPES NumType, char* pResult, size_t LenResult)
{
	int i = 0;
	int mask = 0;
	int rshift = 0;
	int digit = 0;
	char aDigit[2] = { 0 };
	int lastPosRelevant = 0;

	switch (NumType)
	{
	case NUT_BINARY:
		mask = 0x01;
		rshift = 1;
		break;

	case NUT_DUAL:
		mask = 0x03;
		rshift = 2;
		break;

	case NUT_OCTAL:
		mask = 0x07;
		rshift = 3;
		break;

	case NUT_DECIMAL:
		snprintf(pResult, LenResult, "%llu", Number);
		return OK;

	case NUT_HEXADECIMAL:
		snprintf(pResult, LenResult, "%llX", Number);
		return OK;

	default:
		return ERROR;
		break;
	}

	for (i = 0; i < MIN(sizeof(U64) * 8, LenResult); i++)
	{
		digit = Number & mask;
		snprintf(aDigit, ARRAYSIZE(aDigit), "%d", digit);
		Number >>= rshift;

		if (aDigit[0] != '0')
			lastPosRelevant = i;

		pResult[i] = aDigit[0];
	}

	pResult[lastPosRelevant + 1] = '\0';

	CCore::StringRevert(pResult);

	return OK;
}

CMainLogic::E_USERANSWERS CMainLogic::GetUserAnswer(const char *pQuestion, ...)
{
	va_list argptr;
	int retval = 0;
	char aInput[CMAINLOGIC_CONSOLE_USERANSWER_BUFFERLEN] = { 0 };
	char aQuestion[CLOG_LOG_MAXLEN] = { 0 };

	va_start(argptr, pQuestion);
	vsprintf(aQuestion, pQuestion, argptr);
	va_end(argptr);

	// ask question and parse answer loop
	while (1)
	{
		// ask question
		m_Log.LogCustom("", "? (y/n): ", aQuestion);

		// get user input
		retval = scanf(" %" STRINGIFY_VALUE(CMAINLOGIC_CONSOLE_BUFFERLEN) "[^\n]", aInput);

		if (retval <= 0)
		{
			m_Log.LogErr("Failed to scan input");
			continue;
		}

		// allow y/n/yes/no as answer
		if (CCore::StringCompareNocase(aInput, "y", ARRAYSIZE(aInput)) == 0 || CCore::StringCompareNocase(aInput, "yes", ARRAYSIZE(aInput)) == 0)
			return ANS_YES;
		else if (CCore::StringCompareNocase(aInput, "n", ARRAYSIZE(aInput)) == 0 || CCore::StringCompareNocase(aInput, "no", ARRAYSIZE(aInput)) == 0)
			return ANS_NO;
	}

	return ANS_INVALID;
}

int CMainLogic::LoadSaveData()
{
	int retval = 0;

	// load save file
	retval = m_pSaveFile->LoadSaveFile();
	if (retval != OK)
		return ERROR;

	// apply
	// number prefixes
	strncpy(m_asNumbers[NUT_BINARY].m_aPrefix, m_pSaveFile->m_asSaveKeys[CSaveFile::SK_NUMPREFIX_BINARY].m_aValue, ARRAYSIZE(m_asNumbers[0].m_aPrefix));
	strncpy(m_asNumbers[NUT_DUAL].m_aPrefix, m_pSaveFile->m_asSaveKeys[CSaveFile::SK_NUMPREFIX_DUAL].m_aValue, ARRAYSIZE(m_asNumbers[0].m_aPrefix));
	strncpy(m_asNumbers[NUT_OCTAL].m_aPrefix, m_pSaveFile->m_asSaveKeys[CSaveFile::SK_NUMPREFIX_OCTAL].m_aValue, ARRAYSIZE(m_asNumbers[0].m_aPrefix));
	strncpy(m_asNumbers[NUT_DECIMAL].m_aPrefix, m_pSaveFile->m_asSaveKeys[CSaveFile::SK_NUMPREFIX_DECIMAL].m_aValue, ARRAYSIZE(m_asNumbers[0].m_aPrefix));
	strncpy(m_asNumbers[NUT_HEXADECIMAL].m_aPrefix, m_pSaveFile->m_asSaveKeys[CSaveFile::SK_NUMPREFIX_HEXADECIMAL].m_aValue, ARRAYSIZE(m_asNumbers[0].m_aPrefix));

	// operator prefixes
	strncpy(m_asOperators[OPT_ADD].m_aOperator, m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_ADD].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));
	strncpy(m_asOperators[OPT_SUBTRACT].m_aOperator, m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_SUBTRACT].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));
	strncpy(m_asOperators[OPT_MULTIPLY].m_aOperator, m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_MULTIPLY].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));
	strncpy(m_asOperators[OPT_DIVIDE].m_aOperator, m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_DIVIDE].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));
	strncpy(m_asOperators[OPT_EXPONENTIAL].m_aOperator, m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_EXPONENTIAL].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));
	strncpy(m_asOperators[OPT_MODULO].m_aOperator, m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_MODULO].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));
	strncpy(m_asOperators[OPT_AND].m_aOperator, m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_AND].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));
	strncpy(m_asOperators[OPT_OR].m_aOperator, m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_OR].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));
	strncpy(m_asOperators[OPT_XOR].m_aOperator, m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_XOR].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));
	strncpy(m_asOperators[OPT_INVERT].m_aOperator, m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_INVERT].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));
	strncpy(m_asOperators[OPT_REVERT].m_aOperator, m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_REVERT].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));
	strncpy(m_asOperators[OPT_LSHIFT].m_aOperator, m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_LSHIFT].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));
	strncpy(m_asOperators[OPT_RSHIFT].m_aOperator, m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_RSHIFT].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));
	strncpy(m_asOperators[OPT_BRACKET_OPEN].m_aOperator, m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_BRACKET_OPEN].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));
	strncpy(m_asOperators[OPT_BRACKET_CLOSE].m_aOperator, m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_BRACKET_CLOSE].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));

	// result order
	strncpy(m_aResultOrder, m_pSaveFile->m_asSaveKeys[CSaveFile::SK_RESULTORDER].m_aValue, ARRAYSIZE(m_aResultOrder));

	// result prefix visibility
	strncpy(m_aResultPrefixVis, m_pSaveFile->m_asSaveKeys[CSaveFile::SK_RESPREFIXVIS].m_aValue, ARRAYSIZE(m_aResultPrefixVis));

	// fullscreen
	m_StartFullscreen = atoi(m_pSaveFile->m_asSaveKeys[CSaveFile::SK_STARTFULLSCREEN].m_aValue);

	// auto save
	m_AutoSave = atoi(m_pSaveFile->m_asSaveKeys[CSaveFile::SK_AUTOSAVE].m_aValue);

	// input format
	m_DefaultNumberType = (E_NUMTYPES)atoi(m_pSaveFile->m_asSaveKeys[CSaveFile::SK_INPUTFORMAT].m_aValue);

	return OK;
}

int CMainLogic::SaveSaveData()
{
	int retval = 0;

	// write back
	// number prefixes
	strncpy(m_pSaveFile->m_asSaveKeys[CSaveFile::SK_NUMPREFIX_BINARY].m_aValue, m_asNumbers[NUT_BINARY].m_aPrefix, ARRAYSIZE(m_pSaveFile->m_asSaveKeys[0].m_aValue));
	strncpy(m_pSaveFile->m_asSaveKeys[CSaveFile::SK_NUMPREFIX_DUAL].m_aValue, m_asNumbers[NUT_DUAL].m_aPrefix, ARRAYSIZE(m_pSaveFile->m_asSaveKeys[0].m_aValue));
	strncpy(m_pSaveFile->m_asSaveKeys[CSaveFile::SK_NUMPREFIX_OCTAL].m_aValue, m_asNumbers[NUT_OCTAL].m_aPrefix, ARRAYSIZE(m_pSaveFile->m_asSaveKeys[0].m_aValue));
	strncpy(m_pSaveFile->m_asSaveKeys[CSaveFile::SK_NUMPREFIX_DECIMAL].m_aValue, m_asNumbers[NUT_DECIMAL].m_aPrefix, ARRAYSIZE(m_pSaveFile->m_asSaveKeys[0].m_aValue));
	strncpy(m_pSaveFile->m_asSaveKeys[CSaveFile::SK_NUMPREFIX_HEXADECIMAL].m_aValue, m_asNumbers[NUT_HEXADECIMAL].m_aPrefix, ARRAYSIZE(m_pSaveFile->m_asSaveKeys[0].m_aValue));

	// operator prefixes
	strncpy(m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_ADD].m_aValue, m_asOperators[OPT_ADD].m_aOperator, ARRAYSIZE(m_pSaveFile->m_asSaveKeys[0].m_aValue));
	strncpy(m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_SUBTRACT].m_aValue, m_asOperators[OPT_SUBTRACT].m_aOperator, ARRAYSIZE(m_pSaveFile->m_asSaveKeys[0].m_aValue));
	strncpy(m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_MULTIPLY].m_aValue, m_asOperators[OPT_MULTIPLY].m_aOperator, ARRAYSIZE(m_pSaveFile->m_asSaveKeys[0].m_aValue));
	strncpy(m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_DIVIDE].m_aValue, m_asOperators[OPT_DIVIDE].m_aOperator, ARRAYSIZE(m_pSaveFile->m_asSaveKeys[0].m_aValue));
	strncpy(m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_EXPONENTIAL].m_aValue, m_asOperators[OPT_EXPONENTIAL].m_aOperator, ARRAYSIZE(m_pSaveFile->m_asSaveKeys[0].m_aValue));
	strncpy(m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_MODULO].m_aValue, m_asOperators[OPT_MODULO].m_aOperator, ARRAYSIZE(m_pSaveFile->m_asSaveKeys[0].m_aValue));
	strncpy(m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_AND].m_aValue, m_asOperators[OPT_AND].m_aOperator, ARRAYSIZE(m_pSaveFile->m_asSaveKeys[0].m_aValue));
	strncpy(m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_OR].m_aValue, m_asOperators[OPT_OR].m_aOperator, ARRAYSIZE(m_pSaveFile->m_asSaveKeys[0].m_aValue));
	strncpy(m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_XOR].m_aValue, m_asOperators[OPT_XOR].m_aOperator, ARRAYSIZE(m_pSaveFile->m_asSaveKeys[0].m_aValue));
	strncpy(m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_INVERT].m_aValue, m_asOperators[OPT_INVERT].m_aOperator, ARRAYSIZE(m_pSaveFile->m_asSaveKeys[0].m_aValue));
	strncpy(m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_REVERT].m_aValue, m_asOperators[OPT_REVERT].m_aOperator, ARRAYSIZE(m_pSaveFile->m_asSaveKeys[0].m_aValue));
	strncpy(m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_LSHIFT].m_aValue, m_asOperators[OPT_LSHIFT].m_aOperator, ARRAYSIZE(m_pSaveFile->m_asSaveKeys[0].m_aValue));
	strncpy(m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_RSHIFT].m_aValue, m_asOperators[OPT_RSHIFT].m_aOperator, ARRAYSIZE(m_pSaveFile->m_asSaveKeys[0].m_aValue));
	strncpy(m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_BRACKET_OPEN].m_aValue, m_asOperators[OPT_BRACKET_OPEN].m_aOperator, ARRAYSIZE(m_pSaveFile->m_asSaveKeys[0].m_aValue));
	strncpy(m_pSaveFile->m_asSaveKeys[CSaveFile::SK_OPPREFIX_BRACKET_CLOSE].m_aValue, m_asOperators[OPT_BRACKET_CLOSE].m_aOperator,		ARRAYSIZE(m_pSaveFile->m_asSaveKeys[0].m_aValue));

	// result order
	strncpy(m_pSaveFile->m_asSaveKeys[CSaveFile::SK_RESULTORDER].m_aValue, m_aResultOrder, ARRAYSIZE(m_pSaveFile->m_asSaveKeys[0].m_aValue));
	
	// result prefix visibility
	strncpy(m_pSaveFile->m_asSaveKeys[CSaveFile::SK_RESPREFIXVIS].m_aValue, m_aResultPrefixVis, ARRAYSIZE(m_pSaveFile->m_asSaveKeys[0].m_aValue));

	// fullscreen
	snprintf(m_pSaveFile->m_asSaveKeys[CSaveFile::SK_STARTFULLSCREEN].m_aValue, ARRAYSIZE(m_pSaveFile->m_asSaveKeys[0].m_aValue), "%d", m_StartFullscreen);

	// auto save
	snprintf(m_pSaveFile->m_asSaveKeys[CSaveFile::SK_AUTOSAVE].m_aValue, ARRAYSIZE(m_pSaveFile->m_asSaveKeys[0].m_aValue), "%d", m_AutoSave);

	// input format
	snprintf(m_pSaveFile->m_asSaveKeys[CSaveFile::SK_INPUTFORMAT].m_aValue, ARRAYSIZE(m_pSaveFile->m_asSaveKeys[0].m_aValue), "%d", m_DefaultNumberType);

	// save save file
	retval = m_pSaveFile->SaveSaveFile();
	if (retval != OK)
		return ERROR;

	return OK;
}

CMainLogic::E_COMRETVALS CMainLogic::ComHelp(E_COMMANDS ID)
{
	char aParameters[CMAINLOGIC_MAX_LEN_COMHELP_BUFFERS] = { 0 };
	char aExample[CMAINLOGIC_MAX_LEN_COMHELP_BUFFERS] = { 0 };
	S_NUMBER* psNumber = 0;
	S_OPERATOR* psOperator = 0;

	// print full help
	if (ID < 0)
	{
		// HELP
		m_Log.Log(CMAINLOGIC_COMHELP_HEADER_HELP);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "Enter a calculation or one of the commands below.");
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "Max. number size is 64 bit.");
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "All input is case insensitive.");
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "Default input number format (when no prefix is given): %s", GetNumberFromType(m_DefaultNumberType)->m_aName);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "Result order: %s", m_aResultOrder);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "Result prefix visibility: %s", m_aResultPrefixVis);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "Auto saving: %s", m_AutoSave ? "On" : "Off");
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX);

		// COMMANDS
		m_Log.Log(CMAINLOGIC_COMHELP_HEADER_COMMANDS);

		for (int i = 0; i < ARRAYSIZE(m_asCommands); ++i)
		{
			// reset buffers
			memset(aParameters, 0, ARRAYSIZE(aParameters));
			memset(aExample, 0, ARRAYSIZE(aExample));

			// if parameters are given, print parameters
			if (strnlen(m_asCommands[i].m_aParameters, ARRAYSIZE(m_asCommands[i].m_aParameters)) > 0)
				snprintf(aParameters, ARRAYSIZE(aParameters), " Parameters: %s.", m_asCommands[i].m_aParameters);

			// if example is given, print example
			if (strnlen(m_asCommands[i].m_aExample, ARRAYSIZE(m_asCommands[i].m_aExample)) > 0)
				snprintf(aExample, ARRAYSIZE(aExample), " Example: '%s %s'", m_asCommands[i].m_aName, m_asCommands[i].m_aExample);

			// output help
			m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "%s... %s.%s%s", m_asCommands[i].m_aName, m_asCommands[i].m_aDescription, aParameters, aExample);
		}

		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX);

		// NUMPREFIXES
		m_Log.Log(CMAINLOGIC_COMHELP_HEADER_NUMPREFIXES);
		for (int i = (int)NUT_INVALID + 1; i < ARRAYSIZE(m_asNumbers); ++i)
		{
			psNumber = GetNumberFromType((E_NUMTYPES)i);
			m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "%s (short %s)... %s",psNumber->m_aName, psNumber->m_aShortName, psNumber->m_aPrefix);
		}
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX);

		// OPERATORS
		m_Log.Log(CMAINLOGIC_COMHELP_HEADER_OPERATORS);
		for (int i = (int)OPT_INVALID + 1; i < ARRAYSIZE(m_asOperators); ++i)
		{
			psOperator = GetOperatorFromType((E_OPTYPES)i);
			m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "%s... %s", psOperator->m_aName, psOperator->m_aOperator);
		}
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX);

		// TRAILER
		m_Log.Log(CMAINLOGIC_COMHELP_TAILER);
	}
	else// print only the parameters and example of one command
	{
		int index = -1;

		// find command's index
		for (int i = 0; i < ARRAYSIZE(m_asCommands); ++i)
		{
			if (m_asCommands[i].m_ID == ID)
			{
				index = i;
				break;
			}
		}

		if (index >= 0)
		{
			// reset buffers
			memset(aParameters, 0, ARRAYSIZE(aParameters));
			memset(aExample, 0, ARRAYSIZE(aExample));

			// if parameters are given, print parameters
			if (strnlen(m_asCommands[index].m_aParameters, ARRAYSIZE(m_asCommands[index].m_aParameters)) > 0)
				snprintf(aParameters, ARRAYSIZE(aParameters), " Parameters: %s.", m_asCommands[index].m_aParameters);

			// if example is given, print example
			if (strnlen(m_asCommands[index].m_aExample, ARRAYSIZE(m_asCommands[index].m_aExample)) > 0)
				snprintf(aExample, ARRAYSIZE(aExample), " Example: '%s %s'", m_asCommands[index].m_aName, m_asCommands[index].m_aExample);

			// output help
			m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "%s%s", aParameters, aExample);
		}
	}

	return CRV_OK;
}

CMainLogic::E_COMRETVALS CMainLogic::ComSetinputformat(const char *pType)
{
	S_NUMBER* psNumber = 0;

	psNumber = GetNumberFromType(pType);
	if (psNumber->m_NumType == NUT_INVALID)
		return CRV_INVALIDPARAMS;

	m_DefaultNumberType = psNumber->m_NumType;
	m_Log.Log("Default input format is now %s", psNumber->m_aName);

	return CRV_OK;
}

CMainLogic::E_COMRETVALS CMainLogic::ComSetnumberprefix(const char *pType, const char *pNewPrefix)
{
	E_NUMTYPES numberType = NUT_INVALID;
	S_NUMBER* psNumber = 0;
	S_NUMBER* psNumberColliding = 0;
	S_OPERATOR* psOperatorColliding = 0;
	E_USERANSWERS userAnswer = ANS_INVALID;

	psNumber = GetNumberFromType(pType);
	if (psNumber->m_NumType == NUT_INVALID)
		return CRV_INVALIDPARAMS;

	// check empty
	if (CCore::StringIsEmpty(pNewPrefix))
	{
		m_Log.LogErr("Prefix must not be empty");
		return CRV_ERROR;
	}

	// check length
	if (strlen(pNewPrefix) > ARRAYSIZE(psNumber->m_aPrefix))
	{
		m_Log.LogErr("Exceeded max. prefix length of %d", ARRAYSIZE(psNumber->m_aPrefix));
		return CRV_ERROR;
	}

	// check collisions with other number prefixes
	if (CheckNumberPrefixCollisions(pNewPrefix, &psNumberColliding))
	{
		userAnswer = GetUserAnswer("New prefix collides with the %s prefix, do you want to proceed", psNumberColliding->m_aName);
		if (userAnswer != ANS_YES)
			return CRV_CANCELLED;
	}

	// check collisions with other operators
	if (CheckOperatorCollisions(pNewPrefix, &psOperatorColliding))
	{
		userAnswer = GetUserAnswer("New prefix collides with the operator '%s', do you want to proceed", psOperatorColliding->m_aName);
		if (userAnswer != ANS_YES)
			return CRV_CANCELLED;
	}

	// change operator
	memset(psNumber->m_aPrefix, 0, ARRAYSIZE(psNumber->m_aPrefix));
	strncpy(psNumber->m_aPrefix, pNewPrefix, ARRAYSIZE(psNumber->m_aPrefix));
	m_Log.Log("Number prefix for %s is now '%s'", psNumber->m_aName, psNumber->m_aPrefix);

	return CRV_OK;
}

CMainLogic::E_COMRETVALS CMainLogic::ComSetoperator(const char* pType, const char* pNewOperator)
{
	E_OPTYPES operatorType = OPT_INVALID;
	S_OPERATOR* psOperator = 0;
	S_NUMBER* psNumberColliding = 0;
	S_OPERATOR* psOperatorColliding = 0;
	E_USERANSWERS userAnswer = ANS_INVALID;

	psOperator = GetOperatorFromType(pType);
	if (psOperator->m_OpType == NUT_INVALID)
		return CRV_INVALIDPARAMS;

	// check empty
	if (CCore::StringIsEmpty(pNewOperator))
	{
		m_Log.LogErr("Operator must not be empty");
		return CRV_ERROR;
	}

	// check length
	if (strlen(pNewOperator) > ARRAYSIZE(psOperator->m_aOperator))
	{
		m_Log.LogErr("Exceeded max. operator length of %d", ARRAYSIZE(psOperator->m_aOperator));
		return CRV_ERROR;
	}

	// check collisions with other operators
	if (CheckOperatorCollisions(pNewOperator, &psOperatorColliding))
	{
		userAnswer = GetUserAnswer("New operator collides with the operator '%s', do you want to proceed", psOperatorColliding->m_aName);
		if (userAnswer != ANS_YES)
			return CRV_CANCELLED;
	}

	// check collisions with other number prefixes
	if (CheckNumberPrefixCollisions(pNewOperator, &psNumberColliding))
	{
		userAnswer = GetUserAnswer("New operator collides with the %s prefix, do you want to proceed", psNumberColliding->m_aName);
		if (userAnswer != ANS_YES)
			return CRV_CANCELLED;
	}

	// change operator
	memset(psOperator->m_aOperator, 0, ARRAYSIZE(psOperator->m_aOperator));
	strncpy(psOperator->m_aOperator, pNewOperator, ARRAYSIZE(psOperator->m_aOperator));
	m_Log.Log("Operator for %s is now '%s'", psOperator->m_aName, psOperator->m_aOperator);

	return CRV_OK;
}

CMainLogic::E_COMRETVALS CMainLogic::ComSetresultorder(const char* pOrder)
{
	size_t stringLen = strlen(pOrder);
	bool isValid = false;

	// check len
	if (stringLen > ARRAYSIZE(m_aResultOrder))
	{
		m_Log.LogErr("Exceeded max. result order length of %d", ARRAYSIZE(m_aResultOrder));
		return CRV_ERROR;
	}

	// check for invalid characters
	for (int i = 0; i < stringLen; ++i)
	{
		isValid = false;

		for (int s = 0; s < ARRAYSIZE(m_asNumbers); ++s)
		{
			if (CCore::CharCompareNocase(pOrder[i], m_asNumbers[s].m_aShortName[0]) == 0)
			{
				isValid = true;
				break;
			}
		}

		if (!isValid)
		{
			m_Log.LogErr("'%c' is not a valid number format short name", pOrder[i]);
			return CRV_ERROR;
		}
	}

	// apply
	strncpy(m_aResultOrder, pOrder, ARRAYSIZE(m_aResultOrder));
	m_Log.Log("Result order is now '%s'", m_aResultOrder);

	return CRV_OK;
}

CMainLogic::E_COMRETVALS CMainLogic::ComSetresultprefixvisibility(const char* pVisibility)
{
	size_t stringLen = strlen(pVisibility);
	bool isValid = false;

	// check len
	if (stringLen > ARRAYSIZE(m_aResultPrefixVis))
	{
		m_Log.LogErr("Exceeded max. result prefix visibility length of %d", ARRAYSIZE(m_aResultPrefixVis));
		return CRV_ERROR;
	}

	// check for invalid characters
	for (int i = 0; i < stringLen; ++i)
	{
		isValid = false;

		for (int s = 0; s < ARRAYSIZE(m_asNumbers); ++s)
		{
			if (CCore::CharCompareNocase(pVisibility[i], m_asNumbers[s].m_aShortName[0]) == 0)
			{
				isValid = true;
				break;
			}
		}

		if (!isValid)
		{
			m_Log.LogErr("'%c' is not a valid number format short name", pVisibility[i]);
			return CRV_ERROR;
		}
	}

	// apply
	strncpy(m_aResultPrefixVis, pVisibility, ARRAYSIZE(m_aResultPrefixVis));
	m_Log.Log("Result prefix visibility is now '%s'", m_aResultPrefixVis);

	return CRV_OK;
}

CMainLogic::E_COMRETVALS CMainLogic::ComSetautosave(const char* pOption)
{
	size_t stringLen = strlen(pOption);
	bool option = true;
	
	// check option
	if (CCore::StringCompareNocase(pOption, "on", stringLen) == 0)
	{
		option = true;
	}
	else if (CCore::StringCompareNocase(pOption, "off", stringLen) == 0)
	{
		option = false;
	}
	else
	{
		m_Log.LogErr("'%s' is not a valid option", pOption);
		return CRV_ERROR;
	}

	// apply
	m_AutoSave = option;
	m_Log.Log("Auto saving is now %s", pOption);

	return CRV_OK;
}

CMainLogic::E_COMRETVALS CMainLogic::ComSave(const char* pOption)
{
	int retval = 0;

	retval = SaveSaveData();
	if (retval != OK)
		return CRV_ERROR;

	return CRV_OK;
}

CMainLogic::E_COMRETVALS CMainLogic::ComResetSettings(const char* pOption)
{
	E_USERANSWERS userAnswer = ANS_INVALID;
	int retval = 0;

	// let user confirm
	userAnswer = GetUserAnswer("Are you sure");
	if (userAnswer != ANS_YES)
		return CRV_CANCELLED;

	// reset settings
	retval = m_pSaveFile->ResetSaveFile();
	if (retval != OK)
		return CRV_ERROR;

	// apply loaded data
	LoadSaveData();

	m_Log.Log("Settings have been reset");

	return CRV_OK;
}

CMainLogic::E_COMRETVALS CMainLogic::ComClearscreen()
{
	int retval = 0;

	retval = system("cls");
	if (retval != 0)
	{
		m_Log.Log("System command 'cls' returned exit code %d instead of 0", retval);
		return CRV_ERROR;
	}

	return CRV_OK;
}

CMainLogic::E_COMRETVALS CMainLogic::ComExit()
{
	RequestApplicationExit();
	return CRV_OK;
}