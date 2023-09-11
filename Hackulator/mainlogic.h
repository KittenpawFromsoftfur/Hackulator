#pragma once

#include "log.h"
#include "savefile.h"
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

#define CMAINLOGIC_COMSETDEFAULT_BINARY "binary"
#define CMAINLOGIC_COMSETDEFAULT_DUAL "dual"
#define CMAINLOGIC_COMSETDEFAULT_OCTAL "octal"
#define CMAINLOGIC_COMSETDEFAULT_DECIMAL "decimal"
#define CMAINLOGIC_COMSETDEFAULT_HEXADECIMAL "hexadecimal"

#define SIGFLAG_COMBINE (1 << 0)// combining signs such as +, -, *, /, &
#define SIGFLAG_LOGICAL (1 << 1)// logical signs such as &, |
#define SIGFLAG_MODIFYNUM (1 << 2)// number modifying signs such as -, ~
#define SIGFLAG_ALL 0xFFFFFFFFFFFFFFFF// all flags together

#define NUMPREFSTR_BINARY "0b"
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
    CMainLogic(bool StartFullscreen, char *pSaveFilePath);
    ~CMainLogic();
    int EntryPoint();
	void RequestApplicationExit();

	CLog m_Log;
	CSaveFile m_SaveFile;

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
		SIG_MULTIPLY,
		SIG_DIVIDE,
		SIG_EXPONENTIAL,
		SIG_MODULO,
		SIG_AND,
		SIG_OR,
		SIG_XOR,
		SIG_INVERT,
		SIG_REVERT,
		SIG_LSHIFT,
		SIG_RSHIFT,
		SIG_BRACKET_OPEN,
		SIG_BRACKET_CLOSE,
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
		E_COMMANDS m_ID;
		char m_aName[CMAINLOGIC_CONSOLE_TOKEN_SIZE];
		char m_aDescription[CMAINLOGIC_MAX_LEN_COMHELP_BUFFERS];
		char m_aParameters[CMAINLOGIC_MAX_LEN_COMHELP_BUFFERS];
		char m_aExample[CMAINLOGIC_MAX_LEN_COMHELP_BUFFERS];
	} S_COMMAND;

	typedef struct
	{
		E_SIGNS m_Type;
		int m_Flags;
	}S_SIGN;

	typedef struct
	{
		E_NUMBERTYPES m_Type;
		char m_aName[CMAINLOGIC_NUMBERTYPENAMES_LENGTH];
	}S_NUMBERTYPENAME;

	typedef struct
	{
		E_TOKENENTRYTYPES m_Type;
		U64 m_Number;
		E_SIGNS m_Sign;
		char m_aToken[CMAINLOGIC_CONSOLE_TOKEN_SIZE];
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
	const char* GetNumberTypeName(E_NUMBERTYPES Type);
	int ComHelp(E_COMMANDS ID);
	int ComSetdefault(const char *paType);
	int ComClearscreen();

	bool m_StartFullscreen;
	bool m_ExitApplication;
	E_NUMBERTYPES m_DefaultNumberType;
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
	S_NUMBERTYPENAME m_asNumberTypeNames[AMOUNT_NUMBERTYPES] =
	{
		{ NUT_INVALID, "invalid" },
		{ NUT_BINARY, "binary" },
		{ NUT_DUAL, "dual" },
		{ NUT_OCTAL, "octal" },
		{ NUT_DECIMAL, "decimal" },
		{ NUT_HEXADECIMAL, "hexadecimal" },
	};
};

/*
	char gaAsciiSigns1[][32] =
	{
		"NUL (null)",
		"SOH (start of heading)",
		"STX (start of text)",
		"ETX (end of text)",
		"EOT (end of transmission)",
		"ENQ (enquiry)",
		"ACK (acknowledge)",
		"BEL (bell)",
		"BS (backspace)",
		"TAB (horizontal tab)",
		"LF (NL line feed, new line)",
		"VT (vertical tab)",
		"FF (NP form feed, new page)",
		"CR (carriage return)",
		"SO (shift out)",
		"SI (shift in)",
		"DLE (data link escape)",
		"DC1 (device control 1)",
		"DC2 (device control 2)",
		"DC3 (device control 3)",
		"DC4 (device control 4)",
		"NAK (negative acknowledge)",
		"SYN (synchronous idle)",
		"ETB (end of trans. block)",
		"CAN (cancel)",
		"EM (end of medium)",
		"SUB (substitute)",
		"ESC (escape)",
		"FS (file separator)",
		"GS (group separator)",
		"RS (record separator)",
		"US (unit separator)",
		"Space ( )",
	};

	char gaAsciiSigns2[][32] =
	{
		"DEL (delete)",
	};
*/