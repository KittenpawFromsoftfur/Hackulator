#pragma once

#include "log.h"
#include "core.h"

#define CMAINLOGIC_CONSOLE_BUFFERLEN 256
#define CMAINLOGIC_CONSOLE_TOKENS (CMAINLOGIC_CONSOLE_BUFFERLEN / 2)
#define CMAINLOGIC_CONSOLE_TOKEN_SIZE CMAINLOGIC_CONSOLE_BUFFERLEN
#define CMAINLOGIC_CONSOLE_NUMBER_PREFIX_LENGTH 2

#define CMAINLOGIC_MAX_LEN_COMHELP_BUFFERS 256
#define CMAINLOGIC_COMHELP_HEADER_HELP		"<<<<<<<<<<<<<<<<<< HELP >>>>>>>>>>>>>>>>>>"
#define CMAINLOGIC_COMHELP_HEADER_COMMANDS	"<<<<<<<<<<<<<<<< COMMANDS >>>>>>>>>>>>>>>>"
#define CMAINLOGIC_COMHELP_HEADER_OPERATORS	"<<<<<<<<<<<<<<< OPERATORS >>>>>>>>>>>>>>>>"
#define CMAINLOGIC_COMHELP_TAILER			"<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>"
#define CMAINLOGIC_COMHELP_PREFIX "> "

#define CMAINLOGIC_NUMBERTYPENAMES_LENGTH 128

#define CMAINLOGIC_MAX_RANGE_ALLOWED "64"

#define CMAINLOGIC_COMSETDEFAULT_BINARY "binary"
#define CMAINLOGIC_COMSETDEFAULT_DUAL "dual"
#define CMAINLOGIC_COMSETDEFAULT_OCTAL "octal"
#define CMAINLOGIC_COMSETDEFAULT_DECIMAL "decimal"
#define CMAINLOGIC_COMSETDEFAULT_HEXADECIMAL "hexadecimal"

#define SIGFLAG_COMBINE (1 << 0)// combining signs such as +, -, *, /, &
#define SIGFLAG_LOGICAL (1 << 1)// logical signs such as &, |
#define SIGFLAG_MODIFYNUM (1 << 2)// number modifying signs such as -, ~ (tilde = invert)
#define SIGFLAG_ALL 0xFFFFFFFFFFFFFFFF// all flags together

#define SIGNUMFLAG_NEGATED (1 << 0)// negates a number

#define NUMPREFSTR_BINARY "0d"
#define NUMPREFSTR_DUAL "0u"
#define NUMPREFSTR_OCTAL "0o"
#define NUMPREFSTR_DECIMAL "0d"
#define NUMPREFSTR_HEXADECIMAL "0x"

#define SIGSTR_ADD "+"
#define SIGSTR_SUBTRACT "-"
#define SIGSTR_AND "&"
#define SIGSTR_OR "|"
#define SIGSTR_XOR "^"
#define SIGSTR_INVERT "~"
#define SIGSTR_REVERT "<"

class CMainLogic
{
public:
    CMainLogic();
    ~CMainLogic();
    int EntryPoint();
	void RequestApplicationExit();

private:
	enum E_COMMANDS
	{
		COM_HELP,
		COM_SETDEFAULT,
		COM_CLEARSCREEN,
		COM_EXIT,
		AMOUNT_COMMANDS,
	};

	enum E_TOKENENTRYTYPES
	{
		TET_NUMBER,
		TET_SIGN,
	};

	enum E_SIGNS
	{
		SIG_ADD,
		SIG_SUBTRACT,
		SIG_AND,
		SIG_OR,
		SIG_XOR,
		SIG_INVERT,
		SIG_REVERT,
		AMOUNT_SIGNS,
	};

	enum E_NUMBERTYPES
	{
		NUT_INVALID,
		NUT_BINARY,
		NUT_DUAL,
		NUT_OCTAL,
		NUT_DECIMAL,
		NUT_HEXADECIMAL,
		AMOUNT_NUMBERTYPES,
	};

	typedef struct
	{
		E_COMMANDS ID;
		char aName[CMAINLOGIC_CONSOLE_TOKEN_SIZE];
		char aDescription[CMAINLOGIC_MAX_LEN_COMHELP_BUFFERS];
		char aParameters[CMAINLOGIC_MAX_LEN_COMHELP_BUFFERS];
		char aExample[CMAINLOGIC_MAX_LEN_COMHELP_BUFFERS];
	} S_COMMAND;

	typedef struct
	{
		E_SIGNS type;
		int flags;
	}S_SIGN;

	typedef struct
	{
		E_TOKENENTRYTYPES type;
		U64 number;
		E_SIGNS sign;
		char aToken[CMAINLOGIC_CONSOLE_TOKEN_SIZE];
	}S_TOKENENTRY;

	int ParseInput(const char* pInput, size_t LenInput, char* paaToken, size_t AmountTokens, size_t SizeToken);
	int EvaluateTokens(char aaToken[CMAINLOGIC_CONSOLE_TOKENS][CMAINLOGIC_CONSOLE_TOKEN_SIZE]);
	int TokenEntryToNumber(const char *paToken, U64 *pNumber);
	int ExecuteCommand(E_COMMANDS ID, char aaToken[CMAINLOGIC_CONSOLE_TOKENS][CMAINLOGIC_CONSOLE_TOKEN_SIZE]);
	E_SIGNS TokenEntryToSign(const char *paToken);
	int CheckTokenSyntax(S_TOKENENTRY* pasTokenEntries, size_t AmountEntries);
	U64 CalculateTokens(S_TOKENENTRY* pasTokenEntries, size_t AmountEntries);
	void PrintResult(U64 Result);
	U64 ModifyNumberBySign(U64 Number, E_SIGNS Sign);
	int GetSignFlags(E_SIGNS Sign, int SigFlags);
	int GetFlags(int Value, int Flags);
	int ComHelp(E_COMMANDS ID);
	int ComSetdefault(const char *paType);
	int ComClearscreen();

	bool m_ExitApplication;
	E_NUMBERTYPES m_DefaultNumberType;
    CLog m_Log;
	S_COMMAND m_asCommands[AMOUNT_COMMANDS] = 
	{
		{ COM_HELP, "help", "Lists this help", "", "" },
		{ COM_SETDEFAULT, "setdefault", "Sets the default number type", "<" CMAINLOGIC_COMSETDEFAULT_BINARY "/" CMAINLOGIC_COMSETDEFAULT_DUAL "/" CMAINLOGIC_COMSETDEFAULT_OCTAL "/" CMAINLOGIC_COMSETDEFAULT_DECIMAL "/" CMAINLOGIC_COMSETDEFAULT_HEXADECIMAL ">", CMAINLOGIC_COMSETDEFAULT_HEXADECIMAL },
		{ COM_CLEARSCREEN, "clear", "Clears the screen", "", "" },
		{ COM_EXIT, "exit", "Exits the program", "", "" },
	};
	S_SIGN m_asSigns[AMOUNT_SIGNS] =
	{
		{ SIG_ADD, SIGFLAG_COMBINE | SIGFLAG_MODIFYNUM },
		{ SIG_SUBTRACT, SIGFLAG_COMBINE | SIGFLAG_MODIFYNUM },
		{ SIG_AND, SIGFLAG_COMBINE | SIGFLAG_LOGICAL },
		{ SIG_OR, SIGFLAG_COMBINE | SIGFLAG_LOGICAL },
		{ SIG_XOR, SIGFLAG_COMBINE | SIGFLAG_LOGICAL },
		{ SIG_INVERT, SIGFLAG_MODIFYNUM },
		{ SIG_REVERT, SIGFLAG_MODIFYNUM },
	};
	char m_aNumberTypeNames[AMOUNT_NUMBERTYPES][CMAINLOGIC_NUMBERTYPENAMES_LENGTH];// has to be filled in constructor, because if you shift enum variable positions and forget it, have fun finding the bug
};