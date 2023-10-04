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

CMainLogic::CMainLogic(bool StartMaximized, char *pSaveFilePath) : m_SaveFile(this, pSaveFilePath)
{
	int retval = 0;
	char aDefaultValue[ARRAYSIZE(m_SaveFile.m_asSaveKeys[0].m_aDefaultValue)] = { 0 };

	// default initialize members
	m_ExitApplication = false;
	m_DefaultInputType = INT_DECIMAL;
	m_AutoSave = false;
	memset(m_aResultOrder, 0, ARRAYSIZE(m_aResultOrder));
	memset(m_aResultPrefixVis, 0, ARRAYSIZE(m_aResultPrefixVis));

	m_StartMaximized = StartMaximized;
	if (m_StartMaximized)
		ShowWindow(GetConsoleWindow(), SW_MAXIMIZE);

	// set savefile default keys
	m_SaveFile.SetDefaultKey(CSaveFile::SK_INPPREFIX_BINARY,		CMAINLOGIC_DEFAULT_INPPREFIX_BINARY);
	m_SaveFile.SetDefaultKey(CSaveFile::SK_INPPREFIX_DUAL,			CMAINLOGIC_DEFAULT_INPPREFIX_DUAL);
	m_SaveFile.SetDefaultKey(CSaveFile::SK_INPPREFIX_OCTAL,			CMAINLOGIC_DEFAULT_INPPREFIX_OCTAL);
	m_SaveFile.SetDefaultKey(CSaveFile::SK_INPPREFIX_DECIMAL,		CMAINLOGIC_DEFAULT_INPPREFIX_DECIMAL);
	m_SaveFile.SetDefaultKey(CSaveFile::SK_INPPREFIX_HEXADECIMAL,	CMAINLOGIC_DEFAULT_INPPREFIX_HEXADECIMAL);
	m_SaveFile.SetDefaultKey(CSaveFile::SK_INPPREFIX_ASCII,			CMAINLOGIC_DEFAULT_INPPREFIX_ASCII);
	m_SaveFile.SetDefaultKey(CSaveFile::SK_OPPREFIX_ADD,			CMAINLOGIC_DEFAULT_OPLABEL_ADD);
	m_SaveFile.SetDefaultKey(CSaveFile::SK_OPPREFIX_SUBTRACT,		CMAINLOGIC_DEFAULT_OPLABEL_SUBTRACT );
	m_SaveFile.SetDefaultKey(CSaveFile::SK_OPPREFIX_MULTIPLY,		CMAINLOGIC_DEFAULT_OPLABEL_MULTIPLY);
	m_SaveFile.SetDefaultKey(CSaveFile::SK_OPPREFIX_DIVIDE,			CMAINLOGIC_DEFAULT_OPLABEL_DIVIDE);
	m_SaveFile.SetDefaultKey(CSaveFile::SK_OPPREFIX_EXPONENTIAL,	CMAINLOGIC_DEFAULT_OPLABEL_EXPONENTIAL);
	m_SaveFile.SetDefaultKey(CSaveFile::SK_OPPREFIX_MODULO,			CMAINLOGIC_DEFAULT_OPLABEL_MODULO);
	m_SaveFile.SetDefaultKey(CSaveFile::SK_OPPREFIX_AND,			CMAINLOGIC_DEFAULT_OPLABEL_AND);
	m_SaveFile.SetDefaultKey(CSaveFile::SK_OPPREFIX_OR,				CMAINLOGIC_DEFAULT_OPLABEL_OR);
	m_SaveFile.SetDefaultKey(CSaveFile::SK_OPPREFIX_XOR,			CMAINLOGIC_DEFAULT_OPLABEL_XOR);
	m_SaveFile.SetDefaultKey(CSaveFile::SK_OPPREFIX_LSHIFT,			CMAINLOGIC_DEFAULT_OPLABEL_LSHIFT);
	m_SaveFile.SetDefaultKey(CSaveFile::SK_OPPREFIX_RSHIFT,			CMAINLOGIC_DEFAULT_OPLABEL_RSHIFT);
	m_SaveFile.SetDefaultKey(CSaveFile::SK_OPPREFIX_INVERT,			CMAINLOGIC_DEFAULT_OPLABEL_INVERT);
	m_SaveFile.SetDefaultKey(CSaveFile::SK_OPPREFIX_REVERT,			CMAINLOGIC_DEFAULT_OPLABEL_REVERT);
	m_SaveFile.SetDefaultKey(CSaveFile::SK_OPPREFIX_BRACKET_OPEN,	CMAINLOGIC_DEFAULT_OPLABEL_BRACKET_OPEN);
	m_SaveFile.SetDefaultKey(CSaveFile::SK_OPPREFIX_BRACKET_CLOSE,	CMAINLOGIC_DEFAULT_OPLABEL_BRACKET_CLOSE);
	m_SaveFile.SetDefaultKey(CSaveFile::SK_RESULTORDER,				CMAINLOGIC_DEFAULT_RESULTORDER);
	m_SaveFile.SetDefaultKey(CSaveFile::SK_RESPREFIXVIS,			CMAINLOGIC_DEFAULT_RESPREFIXVIS);
	m_SaveFile.SetDefaultKey(CSaveFile::SK_AUTOSAVE,				CMAINLOGIC_DEFAULT_AUTOSAVE);
	snprintf(aDefaultValue, ARRAYSIZE(aDefaultValue), "%d", INT_DECIMAL);
	m_SaveFile.SetDefaultKey(CSaveFile::SK_INPUTFORMAT,				aDefaultValue);

	// initialize save file
	retval = m_SaveFile.Init();
	if (retval != OK)
		CCore::Exit(EXITCODE_ERR_MAINLOGIC);

	// load save data (overwrites some members)
	retval = LoadSaveData();
	if (retval != OK)
		CCore::Exit(EXITCODE_ERR_MAINLOGIC);

	// coding error checks
	// operator and input names must not contain spaces, so commands like renaming work properly on them
	for (int i = 0; i < ARRAYSIZE(m_asInputs); ++i)
	{
		if (CCore::StringFindFirstCharacter(m_asInputs[i].m_aLabel, " \t") != 0)
		{
			m_Log.LogErr("Input label '%s' contains whitespaces", m_asInputs[i].m_aLabel);
			CCore::Exit(EXITCODE_ERR_MAINLOGIC);
		}
	}

	for (int i = 0; i < ARRAYSIZE(m_asOperators); ++i)
	{
		if (CCore::StringFindFirstCharacter(m_asOperators[i].m_aLabel, " \t") != 0)
		{
			m_Log.LogErr("Operator label '%s' contains whitespaces", m_asOperators[i].m_aLabel);
			CCore::Exit(EXITCODE_ERR_MAINLOGIC);
		}
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

		// split input
		retval = TokenizeInput(aInput, ARRAYSIZE(aInput), &sInputTokens);
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

int CMainLogic::TokenizeInput(const char* pInput, size_t LenInput, S_INPUTTOKENS *psInputTokens)
{
	char aTokenBuf[ARRAYSIZE(psInputTokens->m_aaInputTokens[0])] = { 0 };
	char aTempBuf[ARRAYSIZE(aTokenBuf)] = { 0 };
	bool stringEnd = false;
	bool charIsWhitespace = false;
	bool finishToken = false;
	bool doResetPointer = false;
	int operatorLen = 0;
	int tokenBufLen = 0;
	int tokenIndex = 0;
	bool containsASCIIPrefix = false;
	int charIndexAfterASCIIPrefix = 0;
	bool appendChar = false;

	for (int i = 0; i < LenInput; ++i)
	{
		finishToken = false;
		doResetPointer = false;
		appendChar = false;

		if (!pInput[i] || i >= LenInput - 1)
			stringEnd = true;

		// determine if char is delimiter
		if (pInput[i] == ' ' || pInput[i] == '\t')
			charIsWhitespace = true;
		else
			charIsWhitespace = false;

		// copy char for char to current token buffer
		// check if ASCII prefix is given
		if (CCore::StringContainsNocase(aTokenBuf, GetInputFromType(INT_ASCII)->m_aPrefix) != 0)
		{
			containsASCIIPrefix = true;
		}
		else
		{
			containsASCIIPrefix = false;
			charIndexAfterASCIIPrefix = 0;
		}

		// the first character immediately after the ASCII prefix has to be appended and finishes the token
		if (containsASCIIPrefix && charIndexAfterASCIIPrefix == 0)
		{
			appendChar = true;
			finishToken = true;
		}

		// whitespaces will finish the token without being appeded
		if (charIsWhitespace)
		{
			// but only if token buffer is not empty
			if (strnlen(aTokenBuf, ARRAYSIZE(aTokenBuf)) > 0)
				finishToken = true;
		}
		else// non-whitespace characters are always copied to buffer
		{
			appendChar = true;
		}

		// append char to token buffer
		if (appendChar)
		{
			snprintf(aTempBuf, ARRAYSIZE(aTempBuf), "%s%c", aTokenBuf, pInput[i]);
			strncpy_s(aTokenBuf, aTempBuf, ARRAYSIZE(aTokenBuf));
		}

		// if token is not yet finished, check for operators as delimiters
		if (!finishToken)
		{
			// operators
			for (int opInd = 0; opInd < ARRAYSIZE(m_asOperators); ++opInd)
			{
				if (CCore::StringContainsNocase(aTokenBuf, m_asOperators[opInd].m_aOperator) != 0)
				{
					finishToken = true;
					operatorLen = strnlen_s(m_asOperators[opInd].m_aOperator, ARRAYSIZE(m_asOperators[opInd].m_aOperator));
					tokenBufLen = strnlen_s(aTokenBuf, ARRAYSIZE(aTokenBuf));

					// reset pointer if temp buffer is mixed with stuff
					if (operatorLen != tokenBufLen)
						doResetPointer = true;
					break;
				}
			}
		}

		// finish token, also if string has reached the end and token buffer is filled
		if (finishToken || (stringEnd && strnlen(aTokenBuf, ARRAYSIZE(aTokenBuf)) > 0))
		{
			// check if max tokens exceeded
			if (tokenIndex >= ARRAYSIZE(psInputTokens->m_aaInputTokens[0]))
			{
				m_Log.LogErr("Max. tokens exceeded, max. %d", ARRAYSIZE(psInputTokens->m_aaInputTokens[0]));
				return ERROR;
			}

			// reset the pointer by operator length and null terminate token buffer before the operator sequence
			if (doResetPointer)
			{
				i -= operatorLen;

				if (tokenBufLen < operatorLen || tokenBufLen >= ARRAYSIZE(aTokenBuf))
				{
					m_Log.LogErr("Line %d: \"tokenBufLen < operatorLen || tokenBufLen >= ARRAYSIZE(aTokenBuf)\" --> This should not happen", __LINE__);
					return ERROR;
				}

				aTokenBuf[tokenBufLen - operatorLen] = '\0';
			}

			// copy token buffer to current token
			strncpy(psInputTokens->m_aaInputTokens[tokenIndex], aTokenBuf, ARRAYSIZE(psInputTokens->m_aaInputTokens[0]));

			// null buffers
			memset(aTempBuf, 0, ARRAYSIZE(aTempBuf));
			memset(aTokenBuf, 0, ARRAYSIZE(aTokenBuf));
			tokenIndex++;
		}

		// char after ASCII prefix index counter
		if (containsASCIIPrefix)
			charIndexAfterASCIIPrefix++;

		// end of string reached
		if (stringEnd)
			break;
	}

	return OK;
}

int CMainLogic::EvaluateTokens(S_INPUTTOKENS *psInputTokens)
{
	int retval = 0;
	S_TOKEN asToken[ARRAYSIZE(psInputTokens->m_aaInputTokens)];
	S_TOKEN* apsTokenPostfix[ARRAYSIZE(psInputTokens->m_aaInputTokens)] = { 0 };
	int amountTokens = 0;
	U64 result = 0;
	S_INPUT* pCurrentExpectedInput = GetInputFromType(INT_INVALID);

	memset(asToken, 0, ARRAYSIZE(asToken) * sizeof(S_TOKEN));

	// look for command
	for (int i = 0; i < ARRAYSIZE(m_asCommands); ++i)
	{
		// if a command was entered, execute command and leave
		if (CCore::StringCompareNocase(psInputTokens->m_aaInputTokens[0], m_asCommands[i].m_aLabel, ARRAYSIZE(psInputTokens->m_aaInputTokens[0])) == 0)
		{
			retval = ExecuteCommand(&m_asCommands[i], psInputTokens);
			return retval;
		}
	}
	
	// no command
	// parse tokens
	for (int i = 0; i < ARRAYSIZE(psInputTokens->m_aaInputTokens); ++i)
	{
		if (strnlen(psInputTokens->m_aaInputTokens[i], ARRAYSIZE(psInputTokens->m_aaInputTokens[0])) == 0)
			break;

		// convert input to token
		retval = ConvertInputToToken(psInputTokens->m_aaInputTokens[i], &asToken[i]);
		if (retval != OK)
		{
			pCurrentExpectedInput = GetInputFromString(psInputTokens->m_aaInputTokens[i]);

			if (pCurrentExpectedInput->m_InpType == INT_INVALID)
				pCurrentExpectedInput = GetInputFromType(m_DefaultInputType);

			m_Log.LogErr("%d. token '%s' is invalid (expected %s input format)", i + 1, psInputTokens->m_aaInputTokens[i], pCurrentExpectedInput->m_aLabel);
			return ERROR;
		}

		amountTokens++;
	}

	// evaluate syntax
	retval = CheckSyntax(asToken, amountTokens);
	if (retval != OK)
		return ERROR;

	// convert infix notation to postfix notation
	retval = InfixToPostfix(asToken, amountTokens, apsTokenPostfix, ARRAYSIZE(apsTokenPostfix));
	if (retval != OK)
		return ERROR;

	// calculate tokens
	result = Calculate(apsTokenPostfix, ARRAYSIZE(apsTokenPostfix));

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
		retvalCom = ComSetinputformat(psInputTokens->m_aaInputTokens[1]);
		doAutoSave = true;
		break;

	case COM_SET_INPUTPREFIX:
		retvalCom = ComSetinputprefix(psInputTokens->m_aaInputTokens[1], psInputTokens->m_aaInputTokens[2]);
		doAutoSave = true;
		break;

	case COM_SET_OPERATOR:
		retvalCom = ComSetoperator(psInputTokens->m_aaInputTokens[1], psInputTokens->m_aaInputTokens[2]);
		doAutoSave = true;
		break;

	case COM_SET_RESULTORDER:
		retvalCom = ComSetresultorder(psInputTokens->m_aaInputTokens[1]);
		doAutoSave = true;
		break;

	case COM_SET_RESPREFIXVIS:
		retvalCom = ComSetresultprefixvisibility(psInputTokens->m_aaInputTokens[1]);
		doAutoSave = true;
		break;

	case COM_SET_AUTOSAVE:
		retvalCom = ComSetautosave(psInputTokens->m_aaInputTokens[1]);
		doAutoSave = true;
		forceAutoSave = true;// because if you disable it, that would not be saved
		break;

	case COM_SAVE:
		retvalCom = ComSave(psInputTokens->m_aaInputTokens[1]);
		break;

	case COM_RESETSETTINGS:
		retvalCom = ComResetSettings(psInputTokens->m_aaInputTokens[1]);
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

int CMainLogic::InfixToPostfix(S_TOKEN* pasToken, size_t AmountTokens, S_TOKEN **papsTokenPostfix, size_t SizeTokenPostfix)
{
	int retval = 0;
	S_TOKEN* psCurrent = 0;
	S_TOKEN* apsOutput[CMAINLOGIC_CONSOLE_TOKENS] = { 0 };
	S_TOKEN* apsStack[CMAINLOGIC_CONSOLE_TOKENS] = { 0 };
	
	// use shunting yard algorithm to convert infix notation to postfix notation (also "reverse polish notation"). Source of rules: http://csis.pace.edu/~wolf/CS122/infix-postfix.htm<
	for (int i = 0; i < AmountTokens; ++i)
	{
		psCurrent = &pasToken[i];

		// check current token type
		if (psCurrent->m_TokType == TOT_INPUT)
		{
			// Rule 1: Print operands as they arrive
			retval = PushToStackTop(apsOutput, ARRAYSIZE(apsOutput), psCurrent);
			if (retval != OK)
				return ERROR;
		}
		else if (psCurrent->m_TokType == TOT_OPERATOR)
		{
			// Rule 2: If the stack is empty or contains a left parenthesis on top, push the incoming operator onto the stack.
			if (IsStackEmpty(apsStack) || GetStackItemTop(apsStack, ARRAYSIZE(apsStack))[0]->m_psOperator->m_OpType == OPT_BRACKET_OPEN)
			{
				// Intervention: If the current operator is a closing bracket, pop both off the stack
				if (!IsStackEmpty(apsStack) && psCurrent->m_psOperator->m_OpType == OPT_BRACKET_CLOSE)
				{
					retval = PopStack(apsStack, ARRAYSIZE(apsStack), apsOutput, ARRAYSIZE(apsOutput), OPT_BRACKET_OPEN);
					if (retval != OK)
						return ERROR;
				}
				else// no closing bracket --> follow through with rule 2
				{
					retval = PushToStackTop(apsStack, ARRAYSIZE(apsStack), psCurrent);
					if (retval != OK)
						return ERROR;
				}
			}// Rule 3: If the incoming symbol is a left parenthesis, push it on the stack.
			else if (psCurrent->m_psOperator->m_OpType == OPT_BRACKET_OPEN)
			{
				retval = PushToStackTop(apsStack, ARRAYSIZE(apsStack), psCurrent);
				if (retval != OK)
					return ERROR;
			}// Rule 4: If the incoming symbol is a right parenthesis, pop the stack and print the operators until you see a left parenthesis. Discard the pair of parentheses.
			else if (psCurrent->m_psOperator->m_OpType == OPT_BRACKET_CLOSE)
			{
				retval = PopStack(apsStack, ARRAYSIZE(apsStack), apsOutput, ARRAYSIZE(apsOutput), OPT_BRACKET_OPEN);
				if (retval != OK)
					return ERROR;
			}// Rule 5: If the incoming symbol has higher precedence than the top of the stack, push it on the stack.
			else if (psCurrent->m_psOperator->m_OpPrecedence > GetStackItemTop(apsStack, ARRAYSIZE(apsStack))[0]->m_psOperator->m_OpPrecedence)
			{
				retval = PushToStackTop(apsStack, ARRAYSIZE(apsStack), psCurrent);
				if (retval != OK)
					return ERROR;
			}// Rule 6: If the incoming symbol has equal precedence with the top of the stack, use association.
			else if (psCurrent->m_psOperator->m_OpPrecedence == GetStackItemTop(apsStack, ARRAYSIZE(apsStack))[0]->m_psOperator->m_OpPrecedence)
			{
				// Rule 6 (Continued): If the association is left to right, pop and print the top of the stack and then push the incoming operator.
				if (psCurrent->m_psOperator->m_OpAssociativity == OPA_LEFT)
				{
					retval = PopStack(apsStack, ARRAYSIZE(apsStack), apsOutput, ARRAYSIZE(apsOutput));
					if (retval != OK)
						return ERROR;

					retval = PushToStackTop(apsStack, ARRAYSIZE(apsStack), psCurrent);
					if (retval != OK)
						return ERROR;
				}// Rule 6 (Continued): If the association is right to left, push the incoming operator.
				else if (psCurrent->m_psOperator->m_OpAssociativity == OPA_RIGHT)
				{
					retval = PushToStackTop(apsStack, ARRAYSIZE(apsStack), psCurrent);
					if (retval != OK)
						return ERROR;
				}
			}
			else// Rule 7: If the incoming symbol has lower [or equal] precedence than the symbol on the top of the stack, pop the stack and print the top operator. Then test the incoming operator against the new top of stack.
			{
				// Rule 7 Simplified: As long as the incoming symbol has lower or equal precedence than the symbol on the top of the stack, pop the top of the stack to the output. Otherwise push to stack top.
				while (1)
				{
					S_TOKEN** ppsTop = GetStackItemTop(apsStack, ARRAYSIZE(apsStack));

					if ((ppsTop && psCurrent->m_psOperator->m_OpPrecedence <= ppsTop[0]->m_psOperator->m_OpPrecedence))
					{
						retval = PopStack(apsStack, ARRAYSIZE(apsStack), apsOutput, ARRAYSIZE(apsOutput), OPT_INVALID, 1);
						if (retval != OK)
							return ERROR;
					}
					else
					{
						retval = PushToStackTop(apsStack, ARRAYSIZE(apsStack), psCurrent);
						if (retval != OK)
							return ERROR;

						break;
					}
				}
			}
		}
		else
		{
			m_Log.LogErr("%d. token '%s' has invalid token type %d", i, psCurrent->m_aToken, psCurrent->m_TokType);
			return ERROR;
		}
	}

	// Rule 8: At the end of the expression, pop and print all operators on the stack. (No parentheses should remain.)
	if (GetStackSize(apsStack, ARRAYSIZE(apsStack)) > 0)
	{
		retval = PopStack(apsStack, ARRAYSIZE(apsStack), apsOutput, ARRAYSIZE(apsOutput));
		if (retval != OK)
			return ERROR;
	}

	// copy to postfix tokens
	for (int i = 0; i < SizeTokenPostfix; ++i)
	{
		if (!apsOutput[i])
			break;

		papsTokenPostfix[i] = apsOutput[i];
	}

	return OK;
}

int CMainLogic::GetStackSize(S_TOKEN** papsTarget, size_t MaxSize)
{
	for (int i = 0; i < MaxSize; ++i)
	{
		if (!papsTarget[i])
			return i;
	}

	return MaxSize;
}

bool CMainLogic::IsStackEmpty(S_TOKEN** papsStack)
{
	if (papsStack[0])
		return false;

	return true;
}

int CMainLogic::PushToStackTop(S_TOKEN **papsTarget, size_t SizeStack, S_TOKEN* psToken)
{
	int freeIndex = 0;

	for (int i = 0; i < SizeStack; ++i)
	{
		if (!papsTarget[i])
		{
			papsTarget[i] = psToken;
			return OK;
		}
	}

	m_Log.LogErr("No free space found on stack");
	return ERROR;
}

int CMainLogic::PopStack(S_TOKEN** papsSource, size_t SizeSource, S_TOKEN** papsDest, size_t SizeDest, E_OPTYPES OpStopAndDiscard, int PopCount)
{
	S_TOKEN** ppsOutputSlotFree = 0;
	S_TOKEN** ppsSourceItemTop = 0;
	size_t sizeSource = GetStackSize(papsSource, SizeSource);
	int amountPopped = 0;

	if (sizeSource <= 0)
	{
		m_Log.LogErr("Stack size <= 0");
		return ERROR;
	}

	for (int i = sizeSource - 1; i >= 0; --i)
	{
		// stop at the stopping-operator if one is given
		if (OpStopAndDiscard != OPT_INVALID && papsSource[i]->m_psOperator->m_OpType == OpStopAndDiscard)
		{
			// discard current token
			DiscardStackItem(&papsSource[i]);

			// stop popping
			break;
		}

		// transfer from source to output
		ppsSourceItemTop = GetStackItemTop(papsSource, SizeSource);
		if (!ppsSourceItemTop)
		{
			m_Log.LogErr("Getting stack item top for popping stack source %d", i);
			return ERROR;
		}

		ppsOutputSlotFree = GetStackSlotFree(papsDest, SizeDest);
		if (!ppsOutputSlotFree)
		{
			m_Log.LogErr("Getting free destination slot for transferring stack source %d to", i);
			return ERROR;
		}

		*ppsOutputSlotFree = *ppsSourceItemTop;

		// discard top source item
		DiscardStackItem(ppsSourceItemTop);

		amountPopped++;

		// stop popping after first operator
		if (PopCount >= 1 && amountPopped >= PopCount)
			break;
	}

	return OK;
}

CMainLogic::S_TOKEN** CMainLogic::GetStackItemTop(S_TOKEN **papsTarget, size_t SizeStack)
{
	S_TOKEN** ppsExisting = NULL;

	for (int i = 0; i < SizeStack; ++i)
	{
		if (!papsTarget[i])
			return ppsExisting;

		ppsExisting = &papsTarget[i];
	}

	return ppsExisting;
}

CMainLogic::S_TOKEN** CMainLogic::GetStackSlotFree(S_TOKEN **papsTarget, size_t SizeStack)
{
	for (int i = 0; i < SizeStack; ++i)
	{
		if (!papsTarget[i])
			return &papsTarget[i];
	}

	return NULL;
}

int CMainLogic::DiscardStackItem(S_TOKEN** ppsTarget)
{
	// null target
	*ppsTarget = NULL;

	return OK;
}

int CMainLogic::ConvertInputToToken(const char* pToken, S_TOKEN *psToken)
{
	int retval = 0;
	S_INPUT *psInput = GetInputFromType(INT_INVALID);
	S_OPERATOR* psOperator = GetOperatorFromType(OPT_INVALID);
	char aContent[CMAINLOGIC_CONSOLE_TOKEN_SIZE] = { 0 };
	bool wasPrefixed = false;
	
	// copy input without prefix
	psInput = CopyInputWithoutPrefix(pToken, aContent, ARRAYSIZE(aContent), &wasPrefixed);

	// input type is valid --> It could be an input or non-prefixed operator
	if (psInput->m_InpType != INT_INVALID)
	{
		// check the input format
		if (CheckInputFormat(aContent, psInput->m_InpType))
		{
			// valid input --> fill token data
			psToken->m_psInput = psInput;

			if (psInput->m_InpType == INT_ASCII)
				psToken->m_Number = aContent[0];
			else
				psToken->m_Number = strtoull(aContent, NULL, psInput->m_Radix);

			psToken->m_TokType = TOT_INPUT;
			strncpy(psToken->m_aToken, pToken, ARRAYSIZE(psToken->m_aToken));
			return OK;
		}

		// if it was prefixed, it is for sure not an operator
		if (wasPrefixed)
			return ERROR;
	}

	// if it reaches here, it was not a valid input or an unprefixed operator --> check for operator
	psOperator = GetOperatorFromString(pToken);
	if (psOperator->m_OpType == OPT_INVALID)
		return ERROR;

	// valid operator --> fill token data
	psToken->m_psOperator = psOperator;
	psToken->m_TokType = TOT_OPERATOR;
	strncpy(psToken->m_aToken, pToken, ARRAYSIZE(psToken->m_aToken));
	return OK;
}

CMainLogic::S_INPUT* CMainLogic::CopyInputWithoutPrefix(const char *pToken, char *pContent, size_t LenContent, bool *pWasPrefixed)
{
	S_INPUT* psFoundInput = GetInputFromType(INT_INVALID);
	char* pSubstring = 0;
	int prefixLen = 0;

	// default treat as prefixed
	*pWasPrefixed = true;

	for (int i = 0; i < ARRAYSIZE(m_asInputs); ++i)
	{
		// determine input type of prefix
		pSubstring = CCore::StringContainsNocase(pToken, m_asInputs[i].m_aPrefix);
		if (pSubstring != 0)
		{
			psFoundInput = GetInputFromType((E_INPTYPES)i);
			prefixLen = strnlen(psFoundInput->m_aPrefix, ARRAYSIZE(psFoundInput->m_aPrefix));
			break;
		}
	}

	// check if input contained a valid prefix
	if (psFoundInput->m_InpType != INT_INVALID)
	{
		// the prefix is at the beginning of the string as it should be
		if (pSubstring == pToken)
		{
			// copy the content to the buffer without prefix and return
			strncpy(pContent, pSubstring + prefixLen, LenContent);
			return psFoundInput;
		}
		else// input is somewhere in the middle of the string, which is not the correct place
		{
			return GetInputFromType(INT_INVALID);
		}
	}
	else// no prefix was given so copy the entire input and return default number type
	{
		*pWasPrefixed = false;
		strncpy(pContent, pToken, LenContent);
		return GetInputFromType(m_DefaultInputType);
	}
}

int CMainLogic::CheckSyntax(S_TOKEN* pasToken, size_t AmountTokens)
{
	S_TOKEN* psCurrent = 0;
	S_TOKEN* psPrevious = 0;
	int bracketsOpen = 0;
	int bracketsClose = 0;

	for (int i = 0; i < AmountTokens; ++i)
	{
		psCurrent = &pasToken[i];

		if (i > 0)
			psPrevious = &pasToken[i - 1];

		// inputs
		if (psCurrent->m_TokType == TOT_INPUT)
		{
			if (psPrevious)// 2nd+ token, start checking syntax
			{
				// if previous token is an input, input must not be followed by another input
				if (psPrevious->m_TokType == TOT_INPUT)
				{
					m_Log.LogErr("%d. token '%s', input must not be followed by another input", i + 1, psCurrent->m_aToken);
					return ERROR;
				}
			}
		}// operators
		else if (psCurrent->m_TokType == TOT_OPERATOR)
		{
			// bracket checker
			if (GetOpFlags(psCurrent, OPF_ENCASING))
			{
				// bracket counter
				if (GetOpFlags(psCurrent, OPF_ENCASING_START))
				{
					bracketsOpen++;
				}
				else if (GetOpFlags(psCurrent, OPF_ENCASING_STOP))
				{
					// closing bracket only allowed if corresponding opening bracket is given
					if (bracketsOpen <= bracketsClose)
					{
						m_Log.LogErr("%d. token '%s', no corresponding opening bracket", i + 1, psCurrent->m_aToken);
						return ERROR;
					}

					bracketsClose++;
				}
			}

			if (psPrevious)// 2nd+ token is an operator
			{
				// only calculating, combining operators allowed as first token
				if (!(CheckOpFlagsStrict(psCurrent, OPF_CALCULATING | OPF_COMBINING)) && !GetOpFlags(psCurrent, OPF_ENCASING))
				{
					m_Log.LogErr("%d. token '%s', this operator is only allowed as first token", i + 1, psCurrent->m_aToken);
					return ERROR;
				}

				// if previous token was an operator
				if (psPrevious->m_TokType == TOT_OPERATOR)
				{
					// if current operator is calculating
					if (CheckOpFlagsStrict(psCurrent, OPF_CALCULATING))
					{
						// if previous operator was also calculating, calculating operator must not be followed by another calculating operator
						if (CheckOpFlagsStrict(psPrevious, OPF_CALCULATING))
						{
							m_Log.LogErr("%d. token '%s', calculating operator must not be followed by another calculating operator", i + 1, psCurrent->m_aToken);
							return ERROR;
						}
					}

					// if previous operator was an opening bracket, opening bracket can only be followed by another opening bracket
					if (GetOpFlags(psPrevious, OPF_ENCASING_START) && !GetOpFlags(psCurrent, OPF_ENCASING_START))
					{
						m_Log.LogErr("%d. token '%s', opening bracket can only be followed by another opening bracket", i + 1, psCurrent->m_aToken);
						return ERROR;
					}
				}
			}
			else// 1st token is an operator
			{
				// only calculating, non-combining operators allowed as first token
				if (!(CheckOpFlagsStrict(psCurrent, OPF_CALCULATING) && CheckOpFlagsMissing(psCurrent, OPF_COMBINING)) && !GetOpFlags(psCurrent, OPF_ENCASING))
				{
					m_Log.LogErr("%d. token '%s', this operator is not allowed as first token", i + 1, psCurrent->m_aToken);
					return ERROR;
				}
			}
		}
		else
		{
			m_Log.LogErr("%d. token invalid token type", i + 1);
			return ERROR;
		}
	}

	// aftermath
	// brackets must match
	if (bracketsOpen != bracketsClose)
	{
		m_Log.LogErr("not all brackets are matched (%d open, %d close)", bracketsOpen, bracketsClose);
		return ERROR;
	}

	// last token must not be a calculating operator
	if (psCurrent && psCurrent->m_TokType == TOT_OPERATOR && GetOpFlags(psCurrent, OPF_CALCULATING))
	{
		m_Log.LogErr("last token '%s', operator must be followed by input", psCurrent->m_aToken);
		return ERROR;
	}

	return OK;
}

bool CMainLogic::CheckOpFlagsStrict(S_TOKEN *psToken, int OpFlags)
{
	if ((psToken->m_psOperator->m_OpFlags & OpFlags) == OpFlags)
		return true;

	return false;
}

int CMainLogic::GetOpFlags(S_TOKEN* psToken, int OpFlags)
{
	return (psToken->m_psOperator->m_OpFlags & OpFlags);
}

bool CMainLogic::CheckOpFlagsMissing(S_TOKEN *psToken, int OpFlags)
{
	if ((psToken->m_psOperator->m_OpFlags & OpFlags))
		return false;

	return true;
}

U64 CMainLogic::Calculate(S_TOKEN** papsTokenPostfix, size_t SizeTokenPostfix)
{
	int retval = 0;
	int topIndex = 0;
	S_TOKEN* psCurrent = 0;
	S_TOKEN* apsStack[CMAINLOGIC_CONSOLE_TOKENS] = { 0 };
	S_TOKEN* psOp1 = 0;
	S_TOKEN* psOp2 = 0;
	U64 tempResult = 0;

	// necessary because only one token given would result in 0 being the result
	tempResult = papsTokenPostfix[0]->m_Number;

	for (int i = 0; i < SizeTokenPostfix; ++i)
	{
		if (!papsTokenPostfix[i])
			break;

		psCurrent = papsTokenPostfix[i];

		// Iterate over the string from left to right and do the following
		// Rule 1: If the current element is an operand, push it into the stack
		if (psCurrent->m_TokType == TOT_INPUT)
		{
			retval = PushToStackTop(apsStack, ARRAYSIZE(apsStack), psCurrent);
			if (retval != OK)
				return ERROR;
		}// Otherwise, if the current element is an operator, do the following
		else if (psCurrent->m_TokType == TOT_OPERATOR)
		{
			// Rule 2: Once an operator is received, pop the two topmost elements and evaluate them and push the result in the stack again.
			// Rule 2 Continued: Pop an element from stack, let it be op1. Pop another element from stack, let it be op2. Compute the result of op2 / op1, and push it into the stack.
			
			// determine top stack index
			topIndex = -1;

			for (int t = 0; t < ARRAYSIZE(apsStack) - 1; ++t)
			{
				if (!apsStack[t])
				{
					topIndex = t - 1;
					break;
				}
			}

			if (topIndex < 0)
			{
				m_Log.LogErr("failed to determine top calculation stack index");
				return ERROR;
			}
			else if (topIndex < 1)
			{
				if (psCurrent->m_psOperator->m_OpType == OPT_INVERT || psCurrent->m_psOperator->m_OpType == OPT_REVERT)
				{
					psOp1 = apsStack[topIndex];

					tempResult = CombineValues(psOp1, NULL, psCurrent->m_psOperator->m_OpType);

					// remove top operator
					apsStack[topIndex] = 0;
				}
				else
				{
					m_Log.LogErr("only one operator is on the stack, need two");
					return ERROR;
				}
			}
			else
			{
				psOp1 = apsStack[topIndex];
				psOp2 = apsStack[topIndex - 1];

				// remove top operator
				apsStack[topIndex] = 0;

				// calculate result "operator2 <operand> operator1"
				tempResult = CombineValues(psOp1, psOp2, psCurrent->m_psOperator->m_OpType);

				// second from top becomes new operator
				apsStack[topIndex - 1]->m_Number = tempResult;
			}
		}
		else
		{
			m_Log.LogErr("Invalid token type %d", psCurrent->m_TokType);
			return ERROR;
		}
	}

	return tempResult;
}

U64 CMainLogic::CombineValues(S_TOKEN* psOperator1, S_TOKEN* psOperator2, E_OPTYPES OpType)
{
	U64 result = 0;
	U64 num1 = 0;;
	U64 num2 = 0;
	int spaceOccupied = 0;
	int byteSize = 0;
	U64 byteMask = 0;

	if (psOperator1)
		num1 = psOperator1->m_Number;

	if (psOperator2)
		num2 = psOperator2->m_Number;

	switch (OpType)
	{
	case OPT_ADD:
		result = num2 + num1;
		break;

	case OPT_SUBTRACT:
		result = num2 - num1;
		break;

	case OPT_MULTIPLY:
		result = num2 * num1;
		break;

	case OPT_DIVIDE:
		result = num2 / num1;
		break;

	case OPT_EXPONENTIAL:
		result = CCore::PowULL(num2, num1);
		break;

	case OPT_MODULO:
		result = num2 % num1;
		break;

	case OPT_AND:
		result = num2 & num1;
		break;

	case OPT_OR:
		result = num2 | num1;
		break;

	case OPT_XOR:
		result = num2 ^ num1;
		break;

	case OPT_LSHIFT:
		result = num2 << num1;
		break;

	case OPT_RSHIFT:
		result = num2 >> num1;
		break;

	case OPT_INVERT:
		byteSize = CCore::GetByteSize(num1);
		byteMask = (0xFFFFFFFFFFFFFFFF >> (64 - byteSize * 8));
		result = (~num1) & byteMask;
		break;

	case OPT_REVERT:
		byteSize = CCore::GetByteSize(num1);
		result = CCore::NumRevert(num1, byteSize);
		break;
	}

	return result;
}

void CMainLogic::PrintResult(U64 Result)
{
	char aaInputs[AMOUNT_INPTYPES][64 + 1] = { 0 };
	bool prefixVisible = false;
	int currentInputIndex = 0;

	for (int i = INT_INVALID + 1; i < ARRAYSIZE(aaInputs); ++i)
		InputToString(Result, (E_INPTYPES)i, aaInputs[i], ARRAYSIZE(aaInputs[0]));

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

		// get input
		currentInputIndex = INT_INVALID;

		for (int inpInd = 0; inpInd < ARRAYSIZE(m_asInputs); ++inpInd)
		{
			if (m_aResultOrder[i] == m_asInputs[inpInd].m_aShortLabel[0])
			{
				currentInputIndex = inpInd;
				break;
			}
		}

		if (currentInputIndex == INT_INVALID)
		{
			m_Log.LogErr("Invalid result order at position %d '%c'", i + 1, m_aResultOrder[i]);
			return;
		}

		// print
		m_Log.LogCustom("", "", " %s%s ", prefixVisible ? GetInputFromType((E_INPTYPES)currentInputIndex)->m_aPrefix : "", aaInputs[currentInputIndex]);
	}

	m_Log.LogCustom("", "\n", "|");
}

U64 CMainLogic::ModifyInputByOperator(U64 Number, E_OPTYPES OpType)
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

bool CMainLogic::CheckInputPrefixCollisions(const char* pString, S_INPUT** ppsInputColliding)
{
	for (int i = 0; i < ARRAYSIZE(m_asInputs); ++i)
	{
		if (CCore::StringCompareNocase(pString, m_asInputs[i].m_aPrefix, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
		{
			*ppsInputColliding = &m_asInputs[i];
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

	return GetOperatorFromType(OPT_INVALID);
}

CMainLogic::S_OPERATOR* CMainLogic::GetOperatorFromLabel(const char * pLabel)
{
	for (int i = 0; i < ARRAYSIZE(m_asOperators); ++i)
	{
		if (CCore::StringCompareNocase(pLabel, m_asOperators[i].m_aLabel, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
			return &m_asOperators[i];
	}

	return GetOperatorFromType(OPT_INVALID);
}

CMainLogic::S_OPERATOR* CMainLogic::GetOperatorFromString(const char* pString)
{
	for (int i = 0; i < ARRAYSIZE(m_asOperators); ++i)
	{
		if (CCore::StringCompareNocase(pString, m_asOperators[i].m_aOperator, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
			return &m_asOperators[i];
	}

	return GetOperatorFromType(OPT_INVALID);
}

CMainLogic::S_INPUT* CMainLogic::GetInputFromType(E_INPTYPES InpType)
{
	for (int i = 0; i < ARRAYSIZE(m_asInputs); ++i)
	{
		if (InpType == m_asInputs[i].m_InpType)
			return &m_asInputs[i];
	}

	return GetInputFromType(INT_INVALID);
}

CMainLogic::S_INPUT* CMainLogic::GetInputFromLabel(const char *pLabel)
{
	for (int i = 0; i < ARRAYSIZE(m_asInputs); ++i)
	{
		if (CCore::StringCompareNocase(pLabel, m_asInputs[i].m_aLabel, CMAINLOGIC_CONSOLE_TOKEN_SIZE) == 0)
			return &m_asInputs[i];
	}

	return GetInputFromType(INT_INVALID);
}


CMainLogic::S_INPUT* CMainLogic::GetInputFromString(const char *pString)
{
	for (int i = 0; i < ARRAYSIZE(m_asInputs); ++i)
	{
		if (CCore::StringContainsNocase(pString, m_asInputs[i].m_aPrefix) != 0)
			return &m_asInputs[i];
	}

	return GetInputFromType(INT_INVALID);
}

bool CMainLogic::CheckInputFormat(const char* pInput, E_INPTYPES InpType)
{
	switch (InpType)
	{
	case INT_BINARY:
		for (; *pInput; ++pInput)
		{
			if (*pInput < '0' || *pInput > '1')
				return false;
		}
		break;

	case INT_DUAL:
		for (; *pInput; ++pInput)
		{
			if (*pInput < '0' || *pInput > '3')
				return false;
		}
		break;

	case INT_OCTAL:
		for (; *pInput; ++pInput)
		{
			if (*pInput < '0' || *pInput > '7')
				return false;
		}
		break;

	case INT_DECIMAL:
		for (; *pInput; ++pInput)
		{
			if (*pInput < '0' || *pInput > '9')
				return false;
		}
		break;

	case INT_HEXADECIMAL:
		for (; *pInput; ++pInput)
		{
			if (!((*pInput >= '0' && *pInput <= '9') ||
				(*pInput >= 'A' && *pInput <= 'F') ||
				(*pInput >= 'a' && *pInput <= 'f')))
				return false;
		}
		break;

	case INT_ASCII:
		// any character allowed, but only one in total
		if (pInput[1] != '\0')
			return false;
		break;
	}

	return true;
}

int CMainLogic::InputToString(U64 Number, E_INPTYPES InpType, char* pResult, size_t LenResult)
{
	int i = 0;
	int mask = 0;
	int rshift = 0;
	int digit = 0;
	char aDigit[2] = { 0 };
	int lastPosRelevant = 0;
	int byteSize = 0;
	U64 numCopy = Number;

	switch (InpType)
	{
	case INT_BINARY:
		mask = 0x01;
		rshift = 1;
		break;

	case INT_DUAL:
		mask = 0x03;
		rshift = 2;
		break;

	case INT_OCTAL:
		mask = 0x07;
		rshift = 3;
		break;

	case INT_DECIMAL:
		snprintf(pResult, LenResult, "%llu", Number);
		return OK;

	case INT_HEXADECIMAL:
		snprintf(pResult, LenResult, "%llX", Number);
		return OK;

	case INT_ASCII:
		// represent as ASCII character
		if (Number >= 32 && Number <= 126)
		{
			snprintf(pResult, LenResult, "\"%c\"", (char)Number);
		}
		else// represent as special character
		{
			if (Number < 32)
				snprintf(pResult, LenResult, "%s", m_aASCIIDescriptions[Number]);
			else if (Number == 127)
				snprintf(pResult, LenResult, "%s", m_aASCIIDescriptions[33]);
			else
				snprintf(pResult, LenResult, "...");
		}
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

	// binary has automatic byte size and shows leading zeroes
	if (InpType == INT_BINARY)
	{
		byteSize = CCore::GetByteSize(numCopy);
		pResult[byteSize * 8] = '\0';
	}
	else
	{
		pResult[lastPosRelevant + 1] = '\0';
	}

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
	retval = m_SaveFile.LoadSaveFile();
	if (retval != OK)
		return ERROR;

	// apply
	// input prefixes
	strncpy(GetInputFromType(INT_BINARY)->m_aPrefix, m_SaveFile.m_asSaveKeys[CSaveFile::SK_INPPREFIX_BINARY].m_aValue, ARRAYSIZE(m_asInputs[0].m_aPrefix));
	strncpy(GetInputFromType(INT_DUAL)->m_aPrefix, m_SaveFile.m_asSaveKeys[CSaveFile::SK_INPPREFIX_DUAL].m_aValue, ARRAYSIZE(m_asInputs[0].m_aPrefix));
	strncpy(GetInputFromType(INT_OCTAL)->m_aPrefix, m_SaveFile.m_asSaveKeys[CSaveFile::SK_INPPREFIX_OCTAL].m_aValue, ARRAYSIZE(m_asInputs[0].m_aPrefix));
	strncpy(GetInputFromType(INT_DECIMAL)->m_aPrefix, m_SaveFile.m_asSaveKeys[CSaveFile::SK_INPPREFIX_DECIMAL].m_aValue, ARRAYSIZE(m_asInputs[0].m_aPrefix));
	strncpy(GetInputFromType(INT_HEXADECIMAL)->m_aPrefix, m_SaveFile.m_asSaveKeys[CSaveFile::SK_INPPREFIX_HEXADECIMAL].m_aValue, ARRAYSIZE(m_asInputs[0].m_aPrefix));
	strncpy(GetInputFromType(INT_ASCII)->m_aPrefix, m_SaveFile.m_asSaveKeys[CSaveFile::SK_INPPREFIX_ASCII].m_aValue, ARRAYSIZE(m_asInputs[0].m_aPrefix));

	// operator prefixes
	strncpy(GetOperatorFromType(OPT_ADD)->m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_ADD].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));
	strncpy(GetOperatorFromType(OPT_SUBTRACT)->m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_SUBTRACT].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));
	strncpy(GetOperatorFromType(OPT_MULTIPLY)->m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_MULTIPLY].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));
	strncpy(GetOperatorFromType(OPT_DIVIDE)->m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_DIVIDE].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));
	strncpy(GetOperatorFromType(OPT_EXPONENTIAL)->m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_EXPONENTIAL].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));
	strncpy(GetOperatorFromType(OPT_MODULO)->m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_MODULO].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));
	strncpy(GetOperatorFromType(OPT_AND)->m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_AND].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));
	strncpy(GetOperatorFromType(OPT_OR)->m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_OR].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));
	strncpy(GetOperatorFromType(OPT_XOR)->m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_XOR].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));
	strncpy(GetOperatorFromType(OPT_LSHIFT)->m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_LSHIFT].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));
	strncpy(GetOperatorFromType(OPT_RSHIFT)->m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_RSHIFT].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));
	strncpy(GetOperatorFromType(OPT_INVERT)->m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_INVERT].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));
	strncpy(GetOperatorFromType(OPT_REVERT)->m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_REVERT].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));
	strncpy(GetOperatorFromType(OPT_BRACKET_OPEN)->m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_BRACKET_OPEN].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));
	strncpy(GetOperatorFromType(OPT_BRACKET_CLOSE)->m_aOperator, m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_BRACKET_CLOSE].m_aValue, ARRAYSIZE(m_asOperators[0].m_aOperator));

	// result order
	strncpy(m_aResultOrder, m_SaveFile.m_asSaveKeys[CSaveFile::SK_RESULTORDER].m_aValue, ARRAYSIZE(m_aResultOrder));

	// result prefix visibility
	strncpy(m_aResultPrefixVis, m_SaveFile.m_asSaveKeys[CSaveFile::SK_RESPREFIXVIS].m_aValue, ARRAYSIZE(m_aResultPrefixVis));

	// auto save
	m_AutoSave = atoi(m_SaveFile.m_asSaveKeys[CSaveFile::SK_AUTOSAVE].m_aValue);

	// input format
	m_DefaultInputType = (E_INPTYPES)atoi(m_SaveFile.m_asSaveKeys[CSaveFile::SK_INPUTFORMAT].m_aValue);

	return OK;
}

int CMainLogic::SaveSaveData()
{
	int retval = 0;

	// write back
	// input prefixes
	strncpy(m_SaveFile.m_asSaveKeys[CSaveFile::SK_INPPREFIX_BINARY].m_aValue, GetInputFromType(INT_BINARY)->m_aPrefix, ARRAYSIZE(m_SaveFile.m_asSaveKeys[0].m_aValue));
	strncpy(m_SaveFile.m_asSaveKeys[CSaveFile::SK_INPPREFIX_DUAL].m_aValue, GetInputFromType(INT_DUAL)->m_aPrefix, ARRAYSIZE(m_SaveFile.m_asSaveKeys[0].m_aValue));
	strncpy(m_SaveFile.m_asSaveKeys[CSaveFile::SK_INPPREFIX_OCTAL].m_aValue, GetInputFromType(INT_OCTAL)->m_aPrefix, ARRAYSIZE(m_SaveFile.m_asSaveKeys[0].m_aValue));
	strncpy(m_SaveFile.m_asSaveKeys[CSaveFile::SK_INPPREFIX_DECIMAL].m_aValue, GetInputFromType(INT_DECIMAL)->m_aPrefix, ARRAYSIZE(m_SaveFile.m_asSaveKeys[0].m_aValue));
	strncpy(m_SaveFile.m_asSaveKeys[CSaveFile::SK_INPPREFIX_HEXADECIMAL].m_aValue, GetInputFromType(INT_HEXADECIMAL)->m_aPrefix, ARRAYSIZE(m_SaveFile.m_asSaveKeys[0].m_aValue));
	strncpy(m_SaveFile.m_asSaveKeys[CSaveFile::SK_INPPREFIX_ASCII].m_aValue, GetInputFromType(INT_ASCII)->m_aPrefix, ARRAYSIZE(m_SaveFile.m_asSaveKeys[0].m_aValue));

	// operator prefixes
	strncpy(m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_ADD].m_aValue, GetOperatorFromType(OPT_ADD)->m_aOperator, ARRAYSIZE(m_SaveFile.m_asSaveKeys[0].m_aValue));
	strncpy(m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_SUBTRACT].m_aValue, GetOperatorFromType(OPT_SUBTRACT)->m_aOperator, ARRAYSIZE(m_SaveFile.m_asSaveKeys[0].m_aValue));
	strncpy(m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_MULTIPLY].m_aValue, GetOperatorFromType(OPT_MULTIPLY)->m_aOperator, ARRAYSIZE(m_SaveFile.m_asSaveKeys[0].m_aValue));
	strncpy(m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_DIVIDE].m_aValue, GetOperatorFromType(OPT_DIVIDE)->m_aOperator, ARRAYSIZE(m_SaveFile.m_asSaveKeys[0].m_aValue));
	strncpy(m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_EXPONENTIAL].m_aValue, GetOperatorFromType(OPT_EXPONENTIAL)->m_aOperator, ARRAYSIZE(m_SaveFile.m_asSaveKeys[0].m_aValue));
	strncpy(m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_MODULO].m_aValue, GetOperatorFromType(OPT_MODULO)->m_aOperator, ARRAYSIZE(m_SaveFile.m_asSaveKeys[0].m_aValue));
	strncpy(m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_AND].m_aValue, GetOperatorFromType(OPT_AND)->m_aOperator, ARRAYSIZE(m_SaveFile.m_asSaveKeys[0].m_aValue));
	strncpy(m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_OR].m_aValue, GetOperatorFromType(OPT_OR)->m_aOperator, ARRAYSIZE(m_SaveFile.m_asSaveKeys[0].m_aValue));
	strncpy(m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_XOR].m_aValue, GetOperatorFromType(OPT_XOR)->m_aOperator, ARRAYSIZE(m_SaveFile.m_asSaveKeys[0].m_aValue));
	strncpy(m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_LSHIFT].m_aValue, GetOperatorFromType(OPT_LSHIFT)->m_aOperator, ARRAYSIZE(m_SaveFile.m_asSaveKeys[0].m_aValue));
	strncpy(m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_RSHIFT].m_aValue, GetOperatorFromType(OPT_RSHIFT)->m_aOperator, ARRAYSIZE(m_SaveFile.m_asSaveKeys[0].m_aValue));
	strncpy(m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_INVERT].m_aValue, GetOperatorFromType(OPT_INVERT)->m_aOperator, ARRAYSIZE(m_SaveFile.m_asSaveKeys[0].m_aValue));
	strncpy(m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_REVERT].m_aValue, GetOperatorFromType(OPT_REVERT)->m_aOperator, ARRAYSIZE(m_SaveFile.m_asSaveKeys[0].m_aValue));
	strncpy(m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_BRACKET_OPEN].m_aValue, GetOperatorFromType(OPT_BRACKET_OPEN)->m_aOperator, ARRAYSIZE(m_SaveFile.m_asSaveKeys[0].m_aValue));
	strncpy(m_SaveFile.m_asSaveKeys[CSaveFile::SK_OPPREFIX_BRACKET_CLOSE].m_aValue, GetOperatorFromType(OPT_BRACKET_CLOSE)->m_aOperator,		ARRAYSIZE(m_SaveFile.m_asSaveKeys[0].m_aValue));

	// result order
	strncpy(m_SaveFile.m_asSaveKeys[CSaveFile::SK_RESULTORDER].m_aValue, m_aResultOrder, ARRAYSIZE(m_SaveFile.m_asSaveKeys[0].m_aValue));
	
	// result prefix visibility
	strncpy(m_SaveFile.m_asSaveKeys[CSaveFile::SK_RESPREFIXVIS].m_aValue, m_aResultPrefixVis, ARRAYSIZE(m_SaveFile.m_asSaveKeys[0].m_aValue));

	// auto save
	snprintf(m_SaveFile.m_asSaveKeys[CSaveFile::SK_AUTOSAVE].m_aValue, ARRAYSIZE(m_SaveFile.m_asSaveKeys[0].m_aValue), "%d", m_AutoSave);

	// input format
	snprintf(m_SaveFile.m_asSaveKeys[CSaveFile::SK_INPUTFORMAT].m_aValue, ARRAYSIZE(m_SaveFile.m_asSaveKeys[0].m_aValue), "%d", m_DefaultInputType);

	// save save file
	retval = m_SaveFile.SaveSaveFile();
	if (retval != OK)
		return ERROR;

	return OK;
}

CMainLogic::E_COMRETVALS CMainLogic::ComHelp(E_COMMANDS ID)
{
	char aParameters[CMAINLOGIC_MAX_LEN_COMHELP_BUFFERS] = { 0 };
	char aExample[CMAINLOGIC_MAX_LEN_COMHELP_BUFFERS] = { 0 };
	char aOpInfo[CMAINLOGIC_OPINFO_LENGTH] = { 0 };
	char aOpAssoc[12] = { 0 };
	S_INPUT* psInput = 0;
	S_OPERATOR* psOperator = 0;

	// print full help
	if (ID < 0)
	{
		// HELP
		m_Log.Log(CMAINLOGIC_COMHELP_HEADER_HELP);
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "Enter a calculation or one of the commands below.");
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "Input is unsigned 64 bit number.");
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "All input is case insensitive.");
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "Default input format (when no prefix is given): %s", GetInputFromType(m_DefaultInputType)->m_aLabel);
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
				snprintf(aExample, ARRAYSIZE(aExample), " Example: '%s %s'", m_asCommands[i].m_aLabel, m_asCommands[i].m_aExample);

			// output help
			m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "%s... %s.%s%s", m_asCommands[i].m_aLabel, m_asCommands[i].m_aDescription, aParameters, aExample);
		}

		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX);

		// INPPREFIXES
		m_Log.Log(CMAINLOGIC_COMHELP_HEADER_INPPREFIXES);
		for (int i = INT_INVALID + 1; i < ARRAYSIZE(m_asInputs); ++i)
		{
			psInput = GetInputFromType((E_INPTYPES)i);
			m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "%s (short %s)... %s", psInput->m_aLabel, psInput->m_aShortLabel, psInput->m_aPrefix);
		}
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX);

		// OPERATORS
		m_Log.Log(CMAINLOGIC_COMHELP_HEADER_OPERATORS);
		for (int i = OPT_INVALID + 1; i < ARRAYSIZE(m_asOperators); ++i)
		{
			psOperator = GetOperatorFromType((E_OPTYPES)i);
			m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "%s... %s", psOperator->m_aLabel, psOperator->m_aOperator);
		}
		m_Log.Log(CMAINLOGIC_COMHELP_PREFIX);

		// OPERATOR DETAILS
		m_Log.Log(CMAINLOGIC_COMHELP_HEADER_OPERATORDETAILS);
		for (int i = OPT_INVALID + 1; i < ARRAYSIZE(m_asOperators); ++i)
		{
			psOperator = GetOperatorFromType((E_OPTYPES)i);

			if (psOperator->m_OpAssociativity == OPA_LEFT)
				snprintf(aOpAssoc, ARRAYSIZE(aOpAssoc), "left");
			else if (psOperator->m_OpAssociativity == OPA_RIGHT)
				snprintf(aOpAssoc, ARRAYSIZE(aOpAssoc), "right");
			else if (psOperator->m_OpAssociativity == OPA_NONE)
				snprintf(aOpAssoc, ARRAYSIZE(aOpAssoc), "none");
			else
				snprintf(aOpAssoc, ARRAYSIZE(aOpAssoc), "invalid");

			memset(aOpInfo, 0, ARRAYSIZE(aOpInfo));
			if (strnlen(psOperator->m_aInfo, ARRAYSIZE(psOperator->m_aInfo)) > 0)
				snprintf(aOpInfo, ARRAYSIZE(aOpInfo), " (%s)", psOperator->m_aInfo);

			m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "%s...\tprecedence: %d, associativity: %s%s", psOperator->m_aOperator, psOperator->m_OpPrecedence, aOpAssoc, aOpInfo);
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
				snprintf(aExample, ARRAYSIZE(aExample), " Example: '%s %s'", m_asCommands[index].m_aLabel, m_asCommands[index].m_aExample);

			// output help
			m_Log.Log(CMAINLOGIC_COMHELP_PREFIX "%s%s", aParameters, aExample);
		}
	}

	return CRV_OK;
}

CMainLogic::E_COMRETVALS CMainLogic::ComSetinputformat(const char *pType)
{
	S_INPUT* psInput = 0;

	psInput = GetInputFromLabel(pType);
	if (psInput->m_InpType == INT_INVALID)
		return CRV_INVALIDPARAMS;

	m_DefaultInputType = psInput->m_InpType;
	m_Log.Log("Default input format is now %s", psInput->m_aLabel);

	return CRV_OK;
}

CMainLogic::E_COMRETVALS CMainLogic::ComSetinputprefix(const char *pType, const char *pNewPrefix)
{
	E_INPTYPES inputType = INT_INVALID;
	S_INPUT* psInput = 0;
	S_INPUT* psInputColliding = 0;
	S_OPERATOR* psOperatorColliding = 0;
	E_USERANSWERS userAnswer = ANS_INVALID;

	psInput = GetInputFromLabel(pType);
	if (psInput->m_InpType == INT_INVALID)
		return CRV_INVALIDPARAMS;

	// check empty
	if (CCore::StringIsEmpty(pNewPrefix))
	{
		m_Log.LogErr("Prefix must not be empty");
		return CRV_ERROR;
	}

	// check length
	if (strlen(pNewPrefix) > ARRAYSIZE(psInput->m_aPrefix))
	{
		m_Log.LogErr("Exceeded max. prefix length of %d", ARRAYSIZE(psInput->m_aPrefix));
		return CRV_ERROR;
	}

	// check collisions with other input prefixes
	if (CheckInputPrefixCollisions(pNewPrefix, &psInputColliding))
	{
		userAnswer = GetUserAnswer("New prefix collides with the %s prefix, do you want to proceed", psInputColliding->m_aLabel);
		if (userAnswer != ANS_YES)
			return CRV_CANCELLED;
	}

	// check collisions with other operators
	if (CheckOperatorCollisions(pNewPrefix, &psOperatorColliding))
	{
		userAnswer = GetUserAnswer("New prefix collides with the operator '%s', do you want to proceed", psOperatorColliding->m_aLabel);
		if (userAnswer != ANS_YES)
			return CRV_CANCELLED;
	}

	// change operator
	memset(psInput->m_aPrefix, 0, ARRAYSIZE(psInput->m_aPrefix));
	strncpy(psInput->m_aPrefix, pNewPrefix, ARRAYSIZE(psInput->m_aPrefix));
	m_Log.Log("Input prefix for %s is now '%s'", psInput->m_aLabel, psInput->m_aPrefix);

	return CRV_OK;
}

CMainLogic::E_COMRETVALS CMainLogic::ComSetoperator(const char* pLabel, const char* pNewOperator)
{
	E_OPTYPES operatorType = OPT_INVALID;
	S_OPERATOR* psOperator = 0;
	S_INPUT* psInputColliding = 0;
	S_OPERATOR* psOperatorColliding = 0;
	E_USERANSWERS userAnswer = ANS_INVALID;

	psOperator = GetOperatorFromLabel(pLabel);
	if (psOperator->m_OpType == INT_INVALID)
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
		userAnswer = GetUserAnswer("New operator collides with the operator '%s', do you want to proceed", psOperatorColliding->m_aLabel);
		if (userAnswer != ANS_YES)
			return CRV_CANCELLED;
	}

	// check collisions with other input prefixes
	if (CheckInputPrefixCollisions(pNewOperator, &psInputColliding))
	{
		userAnswer = GetUserAnswer("New operator collides with the %s prefix, do you want to proceed", psInputColliding->m_aLabel);
		if (userAnswer != ANS_YES)
			return CRV_CANCELLED;
	}

	// change operator
	memset(psOperator->m_aOperator, 0, ARRAYSIZE(psOperator->m_aOperator));
	strncpy(psOperator->m_aOperator, pNewOperator, ARRAYSIZE(psOperator->m_aOperator));
	m_Log.Log("Operator for %s is now '%s'", psOperator->m_aLabel, psOperator->m_aOperator);

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

		for (int s = 0; s < ARRAYSIZE(m_asInputs); ++s)
		{
			if (CCore::CharCompareNocase(pOrder[i], m_asInputs[s].m_aShortLabel[0]) == 0)
			{
				isValid = true;
				break;
			}
		}

		if (!isValid)
		{
			m_Log.LogErr("'%c' is not a valid input format short name", pOrder[i]);
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

		for (int s = 0; s < ARRAYSIZE(m_asInputs); ++s)
		{
			if (CCore::CharCompareNocase(pVisibility[i], m_asInputs[s].m_aShortLabel[0]) == 0)
			{
				isValid = true;
				break;
			}
		}

		if (!isValid)
		{
			m_Log.LogErr("'%c' is not a valid input format short name", pVisibility[i]);
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

	m_Log.Log("Settings have been saved");

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
	retval = m_SaveFile.ResetSaveFile();
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