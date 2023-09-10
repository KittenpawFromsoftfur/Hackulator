#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <Windows.h>
#include <stdexcept>

#include "mainlogic.h"
#include "core.h"
#include "log.h"

CMainLogic::CMainLogic(bool StartFullscreen, char *pSaveFilePath) : m_SaveFile(this, pSaveFilePath)
{
	int retval = 0;

	m_ExitApplication = false;
	m_DefaultNumberType = NUT_DECIMAL;

	m_StartFullscreen = StartFullscreen;
	if (m_StartFullscreen)
		ShowWindow(GetConsoleWindow(), SW_MAXIMIZE);

	retval = m_SaveFile.Init();
	if (retval != OK)
	{
		CCore::Exit(EXITCODE_ERROR);
	}
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
			return OK;
	
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

    return OK;
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
	bool charIsSymbol = false;
	bool copyToken = false;
	int tokensCopied = 0;

	for (int i = 0; i < LenInput; ++i)
	{
		copyToken = false;

		if (!pInput[i] || i >= LenInput - 1)
			stringEnd = true;

		// determine if char is symbol
		if (CCore::CharContains(pInput[i], SIGSTR_ADD SIGSTR_SUBTRACT SIGSTR_AND SIGSTR_OR SIGSTR_XOR SIGSTR_INVERT SIGSTR_REVERT))
			charIsSymbol = true;
		else
			charIsSymbol = false;

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
		else if (charIsSymbol)// if char is symbol
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
	S_TOKENENTRY asTokenEntries[CMAINLOGIC_CONSOLE_TOKENS];
	int amountEntries = 0;
	U64 result = 0;

	memset(asTokenEntries, 0, ARRAYSIZE(asTokenEntries) * sizeof(S_TOKENENTRY));

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
		if (TokenEntryToNumber(aaToken[i], &asTokenEntries[i].m_Number) == OK)
		{
			asTokenEntries[i].m_Type = TET_NUMBER;
		}// check for signs
		else if ((asTokenEntries[i].m_Sign = TokenEntryToSign(aaToken[i])) != (E_SIGNS)-1)
		{
			asTokenEntries[i].m_Type = TET_SIGN;
		}
		else
		{
			asTokenEntries[i].m_Type = (E_TOKENENTRYTYPES)-1;
		}

		// check token validity and fill rest of token entry data
		if (asTokenEntries[i].m_Type == (E_TOKENENTRYTYPES)-1)
		{
			m_Log.LogErr("%d. token '%s' is an invalid token (expected %s number format)", i + 1, aaToken[i], GetNumberTypeName(m_DefaultNumberType));
			return ERROR;
		}

		strncpy(asTokenEntries[i].m_aToken, aaToken[i], ARRAYSIZE(aaToken[0]));
		amountEntries++;

		//if (asTokenEntries[i].m_Type == TET_NUMBER)
		//	m_Log.Log("--> %llu", asTokenEntries[i].number);
		//else
		//	m_Log.Log("--> %s", asTokenEntries[i].aToken);
	}

	// evaluate syntax
	retval = CheckTokenSyntax(asTokenEntries, amountEntries);
	if (retval != OK)
		return ERROR;

	// calculate tokens
	result = CalculateTokens(asTokenEntries, amountEntries);

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
		break;

	case COM_SETDEFAULT:
		retval = ComSetdefault(aaToken[1]);
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

int CMainLogic::TokenEntryToNumber(const char* paToken, U64 *pNumber)
{
	int retval = 0;
	E_NUMBERTYPES type = NUT_INVALID;
	char aPrefix[CMAINLOGIC_CONSOLE_NUMBER_PREFIX_LENGTH + 1] = { 0 };
	char aContent[CMAINLOGIC_CONSOLE_TOKEN_SIZE] = { 0 };
	bool hasPrefix = true;
	const char* pConvertableInput = aContent;
	int radix = 0;

	if (!pNumber)
		return ERROR;
	
	// check the number type
	// copy prefix
	strncpy(aPrefix, paToken, ARRAYSIZE(aPrefix));
	aPrefix[ARRAYSIZE(aPrefix) -1] = '\0';

	// copy content
	strncpy(aContent, paToken + CMAINLOGIC_CONSOLE_NUMBER_PREFIX_LENGTH, ARRAYSIZE(aContent));

	if (CCore::StringCompareNocase(aPrefix, NUMPREFSTR_BINARY, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
	{
		type = NUT_BINARY;
	}
	else if (CCore::StringCompareNocase(aPrefix, NUMPREFSTR_DUAL, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
	{
		type = NUT_DUAL;
	}
	else if (CCore::StringCompareNocase(aPrefix, NUMPREFSTR_OCTAL, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
	{
		type = NUT_OCTAL;
	}
	else if (CCore::StringCompareNocase(aPrefix, NUMPREFSTR_DECIMAL, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
	{
		type = NUT_DECIMAL;
	}
	else if (CCore::StringCompareNocase(aPrefix, NUMPREFSTR_HEXADECIMAL, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
	{
		type = NUT_HEXADECIMAL;
	}
	else// no prefix given
	{
		hasPrefix = false;
		pConvertableInput = paToken;// if no prefix is given, the convertable input is the token instead of the content which is missing some characters

		// check if non-prefixed input is matching the format of the default type
		switch (m_DefaultNumberType)
		{
		case NUT_BINARY:
			if (CCore::CheckStringFormat(pConvertableInput, CCore::NUMFORM_BINARY))
				type = NUT_BINARY;
			break;

		case NUT_DUAL:
			if (CCore::CheckStringFormat(pConvertableInput, CCore::NUMFORM_DUAL))
				type = NUT_DUAL;
			break;

		case NUT_OCTAL:
			if (CCore::CheckStringFormat(pConvertableInput, CCore::NUMFORM_OCTAL))
				type = NUT_OCTAL;
			break;

		case NUT_DECIMAL:
			if (CCore::CheckStringFormat(pConvertableInput, CCore::NUMFORM_DECIMAL))
				type = NUT_DECIMAL;
			break;

		case NUT_HEXADECIMAL:
			if (CCore::CheckStringFormat(pConvertableInput, CCore::NUMFORM_HEXADECIMAL))
				type = NUT_HEXADECIMAL;
			break;
		}
	}

	// type cannot be determined, exit
	if (type == NUT_INVALID)
		return ERROR;

	// check the number format (only necessary if the input is prefixed, since non-prefixed input has already been checked... see above)
	if (hasPrefix)
	{ 
		switch (type)
		{
		case NUT_BINARY:
			if (!CCore::CheckStringFormat(pConvertableInput, CCore::NUMFORM_BINARY))
				return ERROR;
			break;

		case NUT_DUAL:
			if (!CCore::CheckStringFormat(pConvertableInput, CCore::NUMFORM_DUAL))
				return ERROR;
			break;

		case NUT_OCTAL:
			if (!CCore::CheckStringFormat(pConvertableInput, CCore::NUMFORM_OCTAL))
				return ERROR;
			break;

		case NUT_DECIMAL:
			if (!CCore::CheckStringFormat(pConvertableInput, CCore::NUMFORM_DECIMAL))
				return ERROR;
			break;

		case NUT_HEXADECIMAL:
			if (!CCore::CheckStringFormat(pConvertableInput, CCore::NUMFORM_HEXADECIMAL))
				return ERROR;
			break;
		}
	}

	// convert input to number
	switch (type)
	{
	case NUT_BINARY:
		radix = 2;
		break;

	case NUT_DUAL:
		radix = 4;
		break;

	case NUT_OCTAL:
		radix = 8;
		break;

	case NUT_DECIMAL:
		radix = 10;
		break;

	case NUT_HEXADECIMAL:
		radix = 16;
		break;

	default:
		return ERROR;
	}

	*pNumber = strtoull(pConvertableInput, NULL, radix);

	return OK;
}

CMainLogic::E_SIGNS CMainLogic::TokenEntryToSign(const char* paToken)
{
	E_SIGNS sign = (E_SIGNS)-1;

	if (CCore::CCore::StringCompareNocase(paToken, SIGSTR_ADD, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
	{
		sign = SIG_ADD;
	}
	else if (CCore::StringCompareNocase(paToken, SIGSTR_SUBTRACT, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
	{
		sign = SIG_SUBTRACT;
	}
	else if(CCore::StringCompareNocase(paToken, SIGSTR_AND, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
	{
		sign = SIG_AND;
	}
	else if(CCore::StringCompareNocase(paToken, SIGSTR_OR, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
	{
		sign = SIG_OR;
	}
	else if(CCore::StringCompareNocase(paToken, SIGSTR_XOR, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
	{
		sign = SIG_XOR;
	}
	else if(CCore::StringCompareNocase(paToken, SIGSTR_INVERT, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
	{
		sign = SIG_INVERT;
	}
	else if(CCore::StringCompareNocase(paToken, SIGSTR_REVERT, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
	{
		sign = SIG_REVERT;
	}
	else
	{
		sign = (E_SIGNS)-1;
	}

	return sign;
}

int CMainLogic::CheckTokenSyntax(S_TOKENENTRY* pasTokenEntries, size_t AmountEntries)
{
	int prevSignFlags = 0;
	S_TOKENENTRY *psPreviousEntry = 0;
	S_TOKENENTRY* psCurrentEntry = 0;
	U64 synflags = 0;
	E_SIGNS aSignsNotAllowedFirst[] = { SIG_AND, SIG_OR, SIG_XOR };

	// check token for token
	for (int i = 0; i < AmountEntries; ++i)
	{
		// resets
		psCurrentEntry = &pasTokenEntries[i];
		
		if (i > 0)
			psPreviousEntry = &pasTokenEntries[i - 1];
		
		// check tokens who are signs
		if (psCurrentEntry->m_Type == TET_SIGN)
		{
			// first sign must not be a combining sign
			// however some combining signs double as number-modifying signs, keep that in mind
			if (!psPreviousEntry)
			{
				if ((GetSignFlags(psCurrentEntry->m_Sign, SIGFLAG_COMBINE) > 0) &&
					(GetSignFlags(psCurrentEntry->m_Sign, SIGFLAG_MODIFYNUM) == 0))
				{
					m_Log.LogErr("%d. token '%s', first token must not be a combining sign", i + 1, psCurrentEntry->m_aToken);
					return ERROR;
				}
			}
			else// 2nd+ signs
			{
				// if previous sign was combining or modifies a number, the next token has to be a number
				if ((GetSignFlags(psCurrentEntry->m_Sign, prevSignFlags) & (SIGFLAG_COMBINE || SIGFLAG_MODIFYNUM)) > 0)
				{
					m_Log.LogErr("%d. token '%s', number-combining or number-modifying signs must be followed by a number", i + 1, psCurrentEntry->m_aToken);
					return ERROR;
				}

				// last entry must not be a sign
				if (i >= AmountEntries - 1)
				{
					m_Log.LogErr("%d. token '%s', last token must not be a number-combining or number-modifying sign", i + 1, psCurrentEntry->m_aToken);
					return ERROR;
				}
			}

			// prepare next loop
			prevSignFlags = GetSignFlags(psCurrentEntry->m_Sign, SIGFLAG_ALL);
		}
		else if (psCurrentEntry->m_Type == TET_NUMBER)// check tokens who are numbers
		{
			// first number
			if (!psPreviousEntry)
			{
				// everything allowed
			}
			else// 2nd+ number
			{
				// if previous token is a number, error
				if (psPreviousEntry->m_Type == TET_NUMBER)
				{
					m_Log.LogErr("%d. token '%s', number must not be followed by another number", i + 1, psCurrentEntry->m_aToken);
					return ERROR;
				}
			}

			// prepare next loop
			prevSignFlags = 0;
		}

		// prepare next loop
		psPreviousEntry = psCurrentEntry;
	}

	return OK;
}

U64 CMainLogic::CalculateTokens(S_TOKENENTRY* pasTokenEntries, size_t AmountEntries)
{
	S_TOKENENTRY* psCurrentEntry = 0;
	S_TOKENENTRY asCalculation[CMAINLOGIC_CONSOLE_TOKENS];
	S_TOKENENTRY asTempCalculation[ARRAYSIZE(asCalculation)];
	S_TOKENENTRY sTempTokenEntry;
	bool anyCalculationsLeft = true;
	U64 result = 0;
	int firstNumIndex = 0;
	int firstSignIndex = 0;
	int secondNumIndex = 0;
	int secondSignIndex = 0;
	int amountCurrentEntries = AmountEntries;
	U64 tempResult = 0;
	int amountTokensDisposed = 0;

	// reset structs
	memset(asCalculation, 0, ARRAYSIZE(asCalculation) * sizeof(S_TOKENENTRY));

	// copy original token entries
	for (int i = 0; i < AmountEntries; ++i)
		asCalculation[i] = pasTokenEntries[i];

	// calculation loop, simple from left to right
	while (1)
	{
		// repeat calculation until only one number is left
		for (int i = 0; i < amountCurrentEntries; ++i)
		{
			// reset values
			memset(asTempCalculation, 0, ARRAYSIZE(asTempCalculation) * sizeof(S_TOKENENTRY));
			firstNumIndex = -1;
			firstSignIndex = -1;
			secondNumIndex = -1;
			secondSignIndex = -1;
			amountTokensDisposed = 0;

			// determine number and sign indices
			for (int index = 0; index < amountCurrentEntries; ++index)
			{
				// determine first num index if not yet found
				if (firstNumIndex < 0)
				{
					if (asCalculation[index].m_Type == TET_NUMBER)
					{
						firstNumIndex = index;

						// see if a sign is put in front
						if (index > 0)
							firstSignIndex = index - 1;
					}
				}
				else if (secondNumIndex < 0)// determine second num index if not yet found
				{
					if (index != firstNumIndex && asCalculation[index].m_Type == TET_NUMBER)
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
				E_SIGNS firstSignType = (E_SIGNS)-1;
				E_SIGNS secondSignType = (E_SIGNS)-1;

				// sign types (first one does not have to have a sign)
				if (firstSignIndex >= 0)
					firstSignType = asCalculation[firstSignIndex].m_Sign;

				secondSignType = asCalculation[secondSignIndex].m_Sign;

				// modify numbers
				firstNum = ModifyNumberBySign(firstNum, firstSignType);
				//secondNum = ModifyNumberBySign(secondNum, secondSignType);

				// calculate
				switch (secondSignType)
				{
				case SIG_ADD:
					tempResult = firstNum + secondNum;
					break;

				case SIG_SUBTRACT:
					tempResult = firstNum - secondNum;
					break;

				case SIG_AND:
					tempResult = firstNum & secondNum;
					break;

				case SIG_OR:
					tempResult = firstNum | secondNum;
					break;

				case SIG_XOR:
					tempResult = firstNum ^ secondNum;
					break;

				case SIG_INVERT:
					tempResult = firstNum + secondNum;
					break;

				//case SIG_REVERT:
				//	tempResult = firstNum + secondNum;
				//	break;
				}

				break;
			}
		}

		// assign new token
		memset(&sTempTokenEntry, 0, sizeof(sTempTokenEntry));

		sTempTokenEntry.m_Number = tempResult;
		sTempTokenEntry.m_Type = TET_NUMBER;

		// crop calculation
		asTempCalculation[0] = sTempTokenEntry;

		for (int temp = 1; temp < (amountCurrentEntries - amountTokensDisposed); ++temp)
		{
			asTempCalculation[temp] = asCalculation[temp + amountTokensDisposed - 1];
		}

		memcpy(asCalculation, asTempCalculation, sizeof(asTempCalculation[0]) * ARRAYSIZE(asTempCalculation));

		amountCurrentEntries -= amountTokensDisposed;

		if (amountCurrentEntries <= 0)
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

	CCore::NumToString(Result, CCore::NUMFORM_BINARY, aNumBin, ARRAYSIZE(aNumBin));
	CCore::NumToString(Result, CCore::NUMFORM_DUAL, aNumDua, ARRAYSIZE(aNumDua));
	CCore::NumToString(Result, CCore::NUMFORM_OCTAL, aNumOct, ARRAYSIZE(aNumOct));
	CCore::NumToString(Result, CCore::NUMFORM_DECIMAL, aNumDec, ARRAYSIZE(aNumDec));
	CCore::NumToString(Result, CCore::NUMFORM_HEXADECIMAL, aNumHex, ARRAYSIZE(aNumHex));

	m_Log.Log("| %s | 0x%s | 0b%s | 0d%s | 0o%s |", aNumDec, aNumHex, aNumBin, aNumDua, aNumOct);
}

U64 CMainLogic::ModifyNumberBySign(U64 Number, E_SIGNS Sign)
{
	U64 result = Number;

	switch (Sign)
	{
	case SIG_SUBTRACT:
		result *= -1;
		break;
	}

	return result;
}

int CMainLogic::GetSignFlags(E_SIGNS Sign, int SigFlags)
{
	S_SIGN *pSign = 0;

	// look for matching sign in array and return flags
	for (int i = 0; i < ARRAYSIZE(m_asSigns); ++i)
	{
		if (m_asSigns[i].m_Type == Sign)
		{
			pSign = &m_asSigns[i];
			return (pSign->m_Flags & SigFlags);
		}
	}

	m_Log.LogErr("This should not occur, sign with index %d was not define in source code", Sign);
	return 0;
}

int CMainLogic::GetFlags(int Value, int Flags)
{
	return (Value | Flags);
}

const char* CMainLogic::GetNumberTypeName(E_NUMBERTYPES Type)
{
	S_NUMBERTYPENAME* psCurrent = NULL;

	for (int i = 0; i < ARRAYSIZE(m_asNumberTypeNames[0].m_aName); ++i)
	{
		psCurrent = &m_asNumberTypeNames[i];

		if (Type == psCurrent->m_Type)
			return psCurrent->m_aName;
	}

	return m_asNumberTypeNames[NUT_INVALID].m_aName;
}

int CMainLogic::ComHelp(E_COMMANDS ID)
{
	char aParameters[CMAINLOGIC_MAX_LEN_COMHELP_BUFFERS] = { 0 };
	char aExample[CMAINLOGIC_MAX_LEN_COMHELP_BUFFERS] = { 0 };

	// print full help
	if (ID < 0)
	{
		m_Log.Log(CMAINLOGIC_COMHELP_HEADER_HELP);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "Enter a calculation or one of the commands below.");
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "Max. number size is 64 bit.");
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "All input is case insensitive.");
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "Default number type is %s (when no prefix is given).", GetNumberTypeName(m_DefaultNumberType));
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "Number prefixes: " NUMPREFSTR_BINARY "=%s, " NUMPREFSTR_DUAL "=%s, " NUMPREFSTR_OCTAL "=%s, " NUMPREFSTR_DECIMAL "=%s, " NUMPREFSTR_HEXADECIMAL "=%s. Example: '" NUMPREFSTR_HEXADECIMAL "DEADBEEF'", GetNumberTypeName(NUT_BINARY), GetNumberTypeName(NUT_DUAL), GetNumberTypeName(NUT_OCTAL), GetNumberTypeName(NUT_DECIMAL), GetNumberTypeName(NUT_HEXADECIMAL));
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX);
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
		m_Log.Log(CMAINLOGIC_COMHELP_HEADER_OPERATORS);


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

int CMainLogic::ComSetdefault(const char *paType)
{
	if (CCore::StringCompareNocase(paType, CMAINLOGIC_COMSETDEFAULT_BINARY, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
		m_DefaultNumberType = NUT_BINARY;
	else if (CCore::StringCompareNocase(paType, CMAINLOGIC_COMSETDEFAULT_DUAL, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
		m_DefaultNumberType = NUT_DUAL;
	else if (CCore::StringCompareNocase(paType, CMAINLOGIC_COMSETDEFAULT_OCTAL, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
		m_DefaultNumberType = NUT_OCTAL;
	else if (CCore::StringCompareNocase(paType, CMAINLOGIC_COMSETDEFAULT_DECIMAL, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
		m_DefaultNumberType = NUT_DECIMAL;
	else if (CCore::StringCompareNocase(paType, CMAINLOGIC_COMSETDEFAULT_HEXADECIMAL, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
		m_DefaultNumberType = NUT_HEXADECIMAL;
	else
		return ERROR;

	m_Log.Log("Default is now %s", GetNumberTypeName(m_DefaultNumberType));

	return OK;
}

int CMainLogic::ComClearscreen()
{
	return system("cls");
}