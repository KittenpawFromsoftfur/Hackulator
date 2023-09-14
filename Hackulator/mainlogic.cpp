#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <Windows.h>
#include <stdexcept>
#include <stdarg.h>

#include "mainlogic.h"
#include "core.h"
#include "log.h"

CMainLogic::CMainLogic(bool StartFullscreen, char *pSaveFilePath) : m_SaveFile(this, pSaveFilePath)
{
	int retval = 0;

	m_ExitApplication = false;
	m_DefaultInputFormat = NUT_DECIMAL;

	m_StartFullscreen = StartFullscreen;
	if (m_StartFullscreen)
		ShowWindow(GetConsoleWindow(), SW_MAXIMIZE);

	// load save file
	retval = m_SaveFile.Load();
	if (retval != OK)
		CCore::Exit(EXITCODE_ERR_MAINLOGIC);

	// apply loaded data
	// number prefixes
	strncpy(m_asNumbers[NUT_BINARY].m_aPrefix, m_SaveFile.m_asSaveKeys[CSaveFile::SK_NUMPREFIX_BINARY].m_aValue, ARRAYSIZE(m_asNumbers[0].m_aPrefix));
	strncpy(m_asNumbers[NUT_DUAL].m_aPrefix, m_SaveFile.m_asSaveKeys[CSaveFile::SK_NUMPREFIX_DUAL].m_aValue, ARRAYSIZE(m_asNumbers[0].m_aPrefix));
	strncpy(m_asNumbers[NUT_OCTAL].m_aPrefix, m_SaveFile.m_asSaveKeys[CSaveFile::SK_NUMPREFIX_OCTAL].m_aValue, ARRAYSIZE(m_asNumbers[0].m_aPrefix));
	strncpy(m_asNumbers[NUT_DECIMAL].m_aPrefix, m_SaveFile.m_asSaveKeys[CSaveFile::SK_NUMPREFIX_DECIMAL].m_aValue, ARRAYSIZE(m_asNumbers[0].m_aPrefix));
	strncpy(m_asNumbers[NUT_HEXADECIMAL].m_aPrefix, m_SaveFile.m_asSaveKeys[CSaveFile::SK_NUMPREFIX_HEXADECIMAL].m_aValue, ARRAYSIZE(m_asNumbers[0].m_aPrefix));

	// operator prefixes
	strncpy(m_asOperators[OPT_ADD].m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_ADD].m_aValue, ARRAYSIZE(m_asNumbers[0].m_aPrefix));
	strncpy(m_asOperators[OPT_SUBTRACT].m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_SUBTRACT].m_aValue, ARRAYSIZE(m_asNumbers[0].m_aPrefix));
	strncpy(m_asOperators[OPT_MULTIPLY].m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_MULTIPLY].m_aValue, ARRAYSIZE(m_asNumbers[0].m_aPrefix));
	strncpy(m_asOperators[OPT_DIVIDE].m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_DIVIDE].m_aValue, ARRAYSIZE(m_asNumbers[0].m_aPrefix));
	strncpy(m_asOperators[OPT_EXPONENTIAL].m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_EXPONENTIAL].m_aValue, ARRAYSIZE(m_asNumbers[0].m_aPrefix));
	strncpy(m_asOperators[OPT_MODULO].m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_MODULO].m_aValue, ARRAYSIZE(m_asNumbers[0].m_aPrefix));
	strncpy(m_asOperators[OPT_AND].m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_AND].m_aValue, ARRAYSIZE(m_asNumbers[0].m_aPrefix));
	strncpy(m_asOperators[OPT_OR].m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_OR].m_aValue, ARRAYSIZE(m_asNumbers[0].m_aPrefix));
	strncpy(m_asOperators[OPT_XOR].m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_XOR].m_aValue, ARRAYSIZE(m_asNumbers[0].m_aPrefix));
	strncpy(m_asOperators[OPT_INVERT].m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_INVERT].m_aValue, ARRAYSIZE(m_asNumbers[0].m_aPrefix));
	strncpy(m_asOperators[OPT_REVERT].m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_REVERT].m_aValue, ARRAYSIZE(m_asNumbers[0].m_aPrefix));
	strncpy(m_asOperators[OPT_LSHIFT].m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_LSHIFT].m_aValue, ARRAYSIZE(m_asNumbers[0].m_aPrefix));
	strncpy(m_asOperators[OPT_RSHIFT].m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_RSHIFT].m_aValue, ARRAYSIZE(m_asNumbers[0].m_aPrefix));
	strncpy(m_asOperators[OPT_BRACKET_OPEN].m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_BRACKET_OPEN].m_aValue, ARRAYSIZE(m_asNumbers[0].m_aPrefix));
	strncpy(m_asOperators[OPT_BRACKET_CLOSE].m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_BRACKET_CLOSE].m_aValue, ARRAYSIZE(m_asNumbers[0].m_aPrefix));
}

CMainLogic::~CMainLogic()
{

}

int CMainLogic::EntryPoint()
{
	int retval = 0;
	char* pToken = 0;
	char* pRest = 0;
	char aInput[CMAINLOGIC_CONSOLE_BUFFERLEN] = { 0 };
	char aaToken[CMAINLOGIC_CONSOLE_TOKENS][CMAINLOGIC_CONSOLE_TOKEN_SIZE] = { { 0 } };
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
		memset(aaToken, 0, ARRAYSIZE(aaToken) * ARRAYSIZE(aaToken[0]));
		tokenIndex = 0;

		// get user input
		retval = scanf(" %" STRINGIFY_VALUE(CMAINLOGIC_CONSOLE_BUFFERLEN) "[^\n]", aInput);

		if (retval <= 0)
		{
			m_Log.LogErr("Failed to scan input");
			continue;
		}

		// parse input
		retval = ParseInput(aInput, ARRAYSIZE(aInput), (char*)&aaToken, ARRAYSIZE(aaToken), ARRAYSIZE(aaToken[0]));
		if (retval != OK)
			continue;

		// evaluate tokens
		EvaluateTokens(aaToken);
	}

    return EXITCODE_OK;
}

void CMainLogic::RequestApplicationExit()
{
	m_ExitApplication = true;
}

int CMainLogic::ParseInput(const char* pInput, size_t LenInput, char *paaToken, size_t AmountTokens, size_t SizeToken)
{
	char* pCurrentToken = paaToken;
	char aTokenBuf[CMAINLOGIC_CONSOLE_TOKEN_SIZE] = { 0 };
	char aTempBuf[ARRAYSIZE(aTokenBuf)] = { 0 };
	bool stringEnd = false;
	bool charIsDelim = false;
	bool charIsOperator = false;
	bool copyToken = false;
	int tokensCopied = 0;

	for (int i = 0; i < LenInput; ++i)
	{
		copyToken = false;

		if (!pInput[i] || i >= LenInput - 1)
			stringEnd = true;

		// determine if char is operator
		charIsOperator = false;

		for (int opInd = 0; opInd < ARRAYSIZE(m_asOperators); ++opInd)
		{
			if (CCore::CharContains(pInput[i], m_asOperators[i].m_aOperator))
			{
				charIsOperator = true;
				break;
			}
		}

		// determine if char is delimiter
		if (pInput[i] == ' ')
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
			if (tokensCopied >= AmountTokens)
			{
				m_Log.LogErr("Max. tokens exceeded, max. %d", AmountTokens);
				return ERROR;
			}

			// copy token buffer to current token
			strncpy(pCurrentToken, aTokenBuf, SizeToken);

			// null buffers
			memset(aTempBuf, 0, ARRAYSIZE(aTempBuf));
			memset(aTokenBuf, 0, ARRAYSIZE(aTokenBuf));

			// prepare next token
			tokensCopied++;
			pCurrentToken += SizeToken;
		}

		if (stringEnd)
			break;
	}

	return OK;
}

int CMainLogic::EvaluateTokens(char aaToken[CMAINLOGIC_CONSOLE_TOKENS][CMAINLOGIC_CONSOLE_TOKEN_SIZE])
{
	int retval = 0;
	S_TOKEN asToken[CMAINLOGIC_CONSOLE_TOKENS];
	int amountTokens = 0;
	U64 result = 0;

	memset(asToken, 0, ARRAYSIZE(asToken) * sizeof(S_TOKEN));

	// look for command
	for (int i = 0; i < ARRAYSIZE(m_asCommands); ++i)
	{
		// if a command was entered, execute command and leave
		if (CCore::StringCompareNocase(aaToken[0], m_asCommands[i].m_aName, ARRAYSIZE(aaToken[0])) == 0)
		{
			return ExecuteCommand(m_asCommands[i].m_ID, aaToken);
		}
	}

	// parse tokens
	for (int i = 0; i < CMAINLOGIC_CONSOLE_TOKENS; ++i)
	{
		if (strnlen(aaToken[i], ARRAYSIZE(aaToken[0])) == 0)
			break;

		// determine token type
		// check for numbers
		if (ExtractNumberFromToken(aaToken[i], &asToken[i].m_Number) == OK)
		{
			asToken[i].m_TokType = TOT_NUMBER;
		}// check for operators
		else if ((asToken[i].m_OpType = GetOperatorFromToken(aaToken[i])->m_OpType) != (E_OPTYPES)-1)
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
			m_Log.LogErr("%d. token '%s' is an invalid token (expected %s number format)", i + 1, aaToken[i], GetNumberFromType(m_DefaultInputFormat)->m_aName);
			return ERROR;
		}

		strncpy(asToken[i].m_aToken, aaToken[i], ARRAYSIZE(aaToken[0]));
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

int CMainLogic::ExecuteCommand(E_COMMANDS ID, char aaToken[CMAINLOGIC_CONSOLE_TOKENS][CMAINLOGIC_CONSOLE_TOKEN_SIZE])
{
	int retval = 0;

	switch (ID)
	{
	case COM_HELP:
		ComHelp((E_COMMANDS) - 1);

		GetUserAnswer("hello %s <%s> you stuupeeed", 33, "heyaa");// laststop
		break;

	case COM_SET_INPUTFORMAT:
		retval = ComSetinputformat(aaToken[1]);
		break;

	case COM_SET_NUMBERPREFIX:
		retval = ComSetnumberprefix(aaToken[1], aaToken[2]);
		break;

	case COM_CLEARSCREEN:
		ComClearscreen();
		break;

	case COM_EXIT:
		RequestApplicationExit();
		break;
	}

	if (retval != OK)
	{
		m_Log.LogErr("Invalid parameters");
		ComHelp(ID);
	}

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
		numType = GetNumberFromType(m_DefaultInputFormat)->m_NumType;
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

CMainLogic::S_OPERATOR* CMainLogic::GetOperatorFromType(E_OPTYPES OpType)
{
	for (int i = 0; i < ARRAYSIZE(m_asOperators); ++i)
	{
		if (OpType == m_asOperators[i].m_OpType)
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

int CMainLogic::CheckSyntax(S_TOKEN* pasToken, size_t AmountTokens)
{
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
		
		// check tokens who are signs
		if (psCurrent->m_TokType == TOT_OPERATOR)
		{
			// first sign must not be a combining sign
			// however some combining signs double as number-modifying signs, keep that in mind
			if (!psPrevious)
			{
				if ((GetOperatorFlags(psCurrent->m_OpType, OPFLAG_COMBINE) > 0) &&
					(GetOperatorFlags(psCurrent->m_OpType, OPFLAG_MODIFYNUM) == 0))
				{
					m_Log.LogErr("%d. token '%s', first token must not be a combining sign", i + 1, psCurrent->m_aToken);
					return ERROR;
				}
			}
			else// 2nd+ signs
			{
				// if previous sign was combining or modifies a number, the next token has to be a number
				if ((GetOperatorFlags(psCurrent->m_OpType, prevOpFlags) & (OPFLAG_COMBINE || OPFLAG_MODIFYNUM)) > 0)
				{
					m_Log.LogErr("%d. token '%s', number-combining or number-modifying signs must be followed by a number", i + 1, psCurrent->m_aToken);
					return ERROR;
				}

				// last token must not be a sign
				if (i >= AmountTokens - 1)
				{
					m_Log.LogErr("%d. token '%s', last token must not be a number-combining or number-modifying sign", i + 1, psCurrent->m_aToken);
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

	return OK;
}

U64 CMainLogic::Calculate(S_TOKEN* pasToken, size_t AmountTokens)
{
	S_TOKEN asCalculation[CMAINLOGIC_CONSOLE_TOKENS];
	S_TOKEN asTempCalculation[ARRAYSIZE(asCalculation)];
	S_TOKEN sTempToken;
	bool anyCalculationsLeft = true;
	U64 result = 0;
	int firstNumIndex = 0;
	int firstSignIndex = 0;
	int secondNumIndex = 0;
	int secondSignIndex = 0;
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
			firstSignIndex = -1;
			secondNumIndex = -1;
			secondSignIndex = -1;
			amountTokensDisposed = 0;

			// determine number and sign indices
			for (int index = 0; index < amountCurrentTokens; ++index)
			{
				// determine first num index if not yet found
				if (firstNumIndex < 0)
				{
					if (asCalculation[index].m_TokType == TOT_NUMBER)
					{
						firstNumIndex = index;

						// see if a sign is put in front
						if (index > 0)
							firstSignIndex = index - 1;
					}
				}
				else if (secondNumIndex < 0)// determine second num index if not yet found
				{
					if (index != firstNumIndex && asCalculation[index].m_TokType == TOT_NUMBER)
					{
						secondNumIndex = index;

						// determine second sign index
						secondSignIndex = index - 1;
					}
				}
				else
				{
					break;
				}
			}

			if (firstNumIndex >= 0)
				amountTokensDisposed++;

			if (firstSignIndex >= 0)
				amountTokensDisposed++;

			if (secondNumIndex >= 0)
				amountTokensDisposed++;

			if (secondSignIndex >= 0)
				amountTokensDisposed++;

			//m_Log.Log("Disposed %d, %d, %d, %d, %d", amountTokensDisposed, firstNumIndex, firstSignIndex, secondNumIndex, secondSignIndex);

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
				E_OPTYPES firstSignType = (E_OPTYPES)-1;
				E_OPTYPES secondSignType = (E_OPTYPES)-1;

				// sign types (first one does not have to have a sign)
				if (firstSignIndex >= 0)
					firstSignType = asCalculation[firstSignIndex].m_OpType;

				secondSignType = asCalculation[secondSignIndex].m_OpType;

				// modify numbers
				firstNum = ModifyNumberByOperator(firstNum, firstSignType);
				//secondNum = ModifyNumberByOperator(secondNum, secondSignType);

				// calculate

				switch (secondSignType)
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
	char aNumBin[256] = { 0 };
	char aNumDua[256] = { 0 };
	char aNumOct[256] = { 0 };
	char aNumDec[256] = { 0 };
	char aNumHex[256] = { 0 };
	int retval = 0;

	NumToString(Result, NUT_BINARY, aNumBin, ARRAYSIZE(aNumBin));
	NumToString(Result, NUT_DUAL, aNumDua, ARRAYSIZE(aNumDua));
	NumToString(Result, NUT_OCTAL, aNumOct, ARRAYSIZE(aNumOct));
	NumToString(Result, NUT_DECIMAL, aNumDec, ARRAYSIZE(aNumDec));
	NumToString(Result, NUT_HEXADECIMAL, aNumHex, ARRAYSIZE(aNumHex));

	m_Log.Log("| %s | 0x%s | 0b%s | 0d%s | 0o%s |", aNumDec, aNumHex, aNumBin, aNumDua, aNumOct);
}

U64 CMainLogic::ModifyNumberByOperator(U64 Number, E_OPTYPES Sign)
{
	U64 result = Number;

	switch (Sign)
	{
	case OPT_SUBTRACT:
		result *= -1;
		break;
	}

	return result;
}

int CMainLogic::GetOperatorFlags(E_OPTYPES Sign, int SigFlags)
{
	S_OPERATOR *pOperator = 0;

	// look for matching sign in array and return flags
	for (int i = 0; i < ARRAYSIZE(m_asOperators); ++i)
	{
		if (m_asOperators[i].m_OpType == Sign)
		{
			pOperator = &m_asOperators[i];
			return (m_asOperators->m_Flags & SigFlags);
		}
	}

	m_Log.LogErr("This should not occur, sign with index %d was not define in source code", Sign);
	return 0;
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
	va_start(argptr, pQuestion);
	int retval = 0;
	char aInput[CMAINLOGIC_CONSOLE_USERANSWER_BUFFERLEN] = { 0 };

	// ask question
	m_Log.LogCustomArg(CLog::CFL_NONEWLINE, pQuestion, argptr);
	va_end(argptr);
	
	// suffix
	m_Log.LogCustom(CLog::CFL_NONEWLINE, "? (y/n): ");

	// parse input loop until question is answered
	while (1)
	{
		// get user input
		retval = scanf(" %" STRINGIFY_VALUE(CMAINLOGIC_CONSOLE_BUFFERLEN) "[^\n]", aInput);

		if (retval <= 0)
		{
			m_Log.LogErr("Failed to scan input");
			continue;
		}
		
		m_Log.Log("User answered <%s>", aInput);
	}

	return ANS_INVALID;
}

int CMainLogic::ComHelp(E_COMMANDS ID)
{
	char aParameters[CMAINLOGIC_MAX_LEN_COMHELP_BUFFERS] = { 0 };
	char aExample[CMAINLOGIC_MAX_LEN_COMHELP_BUFFERS] = { 0 };

	// print full help
	if (ID < 0)
	{
		// HELP
		m_Log.Log(CMAINLOGIC_COMHELP_HEADER_HELP);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "Enter a calculation or one of the commands below.");
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "Max. number size is 64 bit.");
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "All input is case insensitive.");
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "Default number type (when no prefix is given): %s", GetNumberFromType(m_DefaultInputFormat)->m_aName);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "Auto saving: %s", 1 == 1 ? "On" : "Off");
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
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "%s... %s", GetNumberFromType(NUT_BINARY)->m_aName, GetNumberFromType(NUT_BINARY)->m_aPrefix);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "%s... %s", GetNumberFromType(NUT_DUAL)->m_aName, GetNumberFromType(NUT_DUAL)->m_aPrefix);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "%s... %s", GetNumberFromType(NUT_OCTAL)->m_aName, GetNumberFromType(NUT_OCTAL)->m_aPrefix);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "%s... %s", GetNumberFromType(NUT_DECIMAL)->m_aName, GetNumberFromType(NUT_DECIMAL)->m_aPrefix);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "%s... %s", GetNumberFromType(NUT_HEXADECIMAL)->m_aName, GetNumberFromType(NUT_HEXADECIMAL)->m_aPrefix);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX);

		// OPERATORS
		m_Log.Log(CMAINLOGIC_COMHELP_HEADER_OPERATORS);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX	CMAINLOGIC_OPNAME_ADD			"... %s", GetOperatorFromType(OPT_ADD)->m_aOperator);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX	CMAINLOGIC_OPNAME_SUBTRACT		"... %s", GetOperatorFromType(OPT_SUBTRACT)->m_aOperator);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX	CMAINLOGIC_OPNAME_MULTIPLY		"... %s", GetOperatorFromType(OPT_MULTIPLY)->m_aOperator);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX	CMAINLOGIC_OPNAME_DIVIDE		"... %s", GetOperatorFromType(OPT_DIVIDE)->m_aOperator);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX	CMAINLOGIC_OPNAME_EXPONENTIAL	"... %s", GetOperatorFromType(OPT_EXPONENTIAL)->m_aOperator);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX	CMAINLOGIC_OPNAME_MODULO		"... %s", GetOperatorFromType(OPT_MODULO)->m_aOperator);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX	CMAINLOGIC_OPNAME_AND			"... %s", GetOperatorFromType(OPT_AND)->m_aOperator);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX	CMAINLOGIC_OPNAME_OR			"... %s", GetOperatorFromType(OPT_OR)->m_aOperator);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX	CMAINLOGIC_OPNAME_XOR			"... %s", GetOperatorFromType(OPT_XOR)->m_aOperator);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX	CMAINLOGIC_OPNAME_INVERT		"... %s", GetOperatorFromType(OPT_INVERT)->m_aOperator);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX	CMAINLOGIC_OPNAME_REVERT		"... %s", GetOperatorFromType(OPT_REVERT)->m_aOperator);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX	CMAINLOGIC_OPNAME_LSHIFT		"... %s", GetOperatorFromType(OPT_LSHIFT)->m_aOperator);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX	CMAINLOGIC_OPNAME_RSHIFT		"... %s", GetOperatorFromType(OPT_RSHIFT)->m_aOperator);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX	CMAINLOGIC_OPNAME_BRACKET_OPEN	"... %s", GetOperatorFromType(OPT_BRACKET_OPEN)->m_aOperator);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX	CMAINLOGIC_OPNAME_BRACKET_CLOSE	"... %s", GetOperatorFromType(OPT_BRACKET_CLOSE)->m_aOperator);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX);

		// NUMSHORTNAMES
		m_Log.Log(CMAINLOGIC_COMHELP_HEADER_NUMSHORTNAMES);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX CMAINLOGIC_NUMNAME_BINARY		"... " CSAVEFILE_NUMSHORTNAME_BINARY);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX CMAINLOGIC_NUMNAME_DUAL			"... " CSAVEFILE_NUMSHORTNAME_DUAL);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX CMAINLOGIC_NUMNAME_OCTAL		"... " CSAVEFILE_NUMSHORTNAME_OCTAL);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX CMAINLOGIC_NUMNAME_DECIMAL		"... " CSAVEFILE_NUMSHORTNAME_DECIMAL);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX CMAINLOGIC_NUMNAME_HEXADECIMAL	"... " CSAVEFILE_NUMSHORTNAME_HEXADECIMAL);
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
	return OK;
}

int CMainLogic::ComSetinputformat(const char *pType)
{
	if (CCore::StringCompareNocase(pType, CMAINLOGIC_NUMNAME_BINARY, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
		m_DefaultInputFormat = NUT_BINARY;
	else if (CCore::StringCompareNocase(pType, CMAINLOGIC_NUMNAME_DUAL, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
		m_DefaultInputFormat = NUT_DUAL;
	else if (CCore::StringCompareNocase(pType, CMAINLOGIC_NUMNAME_OCTAL, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
		m_DefaultInputFormat = NUT_OCTAL;
	else if (CCore::StringCompareNocase(pType, CMAINLOGIC_NUMNAME_DECIMAL, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
		m_DefaultInputFormat = NUT_DECIMAL;
	else if (CCore::StringCompareNocase(pType, CMAINLOGIC_NUMNAME_HEXADECIMAL, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
		m_DefaultInputFormat = NUT_HEXADECIMAL;
	else
		return ERROR;

	m_Log.Log("Default input format is now %s", pType);

	return OK;
}

int CMainLogic::ComSetnumberprefix(const char *pType, const char *pPrefix)
{
	E_NUMTYPES numberType = NUT_INVALID;

	if (CCore::StringCompareNocase(pType, CMAINLOGIC_NUMNAME_BINARY, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
		numberType = NUT_BINARY;
	else if (CCore::StringCompareNocase(pType, CMAINLOGIC_NUMNAME_DUAL, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
		numberType = NUT_DUAL;
	else if (CCore::StringCompareNocase(pType, CMAINLOGIC_NUMNAME_OCTAL, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
		numberType = NUT_OCTAL;
	else if (CCore::StringCompareNocase(pType, CMAINLOGIC_NUMNAME_DECIMAL, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
		numberType = NUT_DECIMAL;
	else if (CCore::StringCompareNocase(pType, CMAINLOGIC_NUMNAME_HEXADECIMAL, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
		numberType = NUT_HEXADECIMAL;
	else
		return ERROR;

	m_Log.Log("Number prefix for %s is now %s", pType, pPrefix);

	return OK;
}

int CMainLogic::ComClearscreen()
{
	return system("cls");
}
