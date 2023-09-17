#pragma once

#include "log.h"
#include "core.h"

// help
#define CMAINLOGIC_MAX_LEN_COMHELP_BUFFERS 256
#define CMAINLOGIC_COMHELP_HEADER_HELP					"<<<<<<<<<<<<<<<<<<<<<<<<< HELP >>>>>>>>>>>>>>>>>>>>>>>>>"
#define CMAINLOGIC_COMHELP_HEADER_COMMANDS				"<<<<<<<<<<<<<<<<<<<<<<< COMMANDS >>>>>>>>>>>>>>>>>>>>>>>"
#define CMAINLOGIC_COMHELP_HEADER_NUMPREFIXES_TEXT		"NUMBER PREFIXES"
#define CMAINLOGIC_COMHELP_HEADER_NUMPREFIXES			"<<<<<<<<<<<<<<<<<<< " CMAINLOGIC_COMHELP_HEADER_NUMPREFIXES_TEXT " >>>>>>>>>>>>>>>>>>>>"
#define CMAINLOGIC_COMHELP_HEADER_OPERATORS_TEXT		"OPERATORS"
#define CMAINLOGIC_COMHELP_HEADER_OPERATORS				"<<<<<<<<<<<<<<<<<<<<<< " CMAINLOGIC_COMHELP_HEADER_OPERATORS_TEXT " >>>>>>>>>>>>>>>>>>>>>>>"
#define CMAINLOGIC_COMHELP_TAILER						"<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
#define CMAINLOGIC_COMHELP_PREFIX						"> "

// numbers and operators
#define CMAINLOGIC_NUMPREFIXES_LENGTH 12
#define CMAINLOGIC_NUMNAMES_LENGTH 32
#define CMAINLOGIC_NUMSHORTNAMES_LENGTH 2
#define CMAINLOGIC_OPERATORS_LENGTH CMAINLOGIC_NUMPREFIXES_LENGTH
#define CMAINLOGIC_OPNAMES_LENGTH CMAINLOGIC_NUMNAMES_LENGTH
#define CMAINLOGIC_RESULTORDER_LEN 24

// input and tokenization
#define CMAINLOGIC_CONSOLE_BUFFERLEN 256
#define CMAINLOGIC_CONSOLE_TOKENS (CMAINLOGIC_CONSOLE_BUFFERLEN / 2)
#define CMAINLOGIC_CONSOLE_TOKEN_SIZE (64 + CMAINLOGIC_NUMPREFIXES_LENGTH)
#define CMAINLOGIC_CONSOLE_NUMBER_PREFIX_LENGTH 2
#define CMAINLOGIC_CONSOLE_USERANSWER_BUFFERLEN 12

// number names
#define CMAINLOGIC_NUMNAME_BINARY		"binary"
#define CMAINLOGIC_NUMNAME_DUAL			"dual"
#define CMAINLOGIC_NUMNAME_OCTAL		"octal"
#define CMAINLOGIC_NUMNAME_DECIMAL		"decimal"
#define CMAINLOGIC_NUMNAME_HEXADECIMAL	"hexadecimal"

// number short names
#define CMAINLOGIC_NUMSHORTNAME_BINARY       "b"
#define CMAINLOGIC_NUMSHORTNAME_DUAL         "u"
#define CMAINLOGIC_NUMSHORTNAME_OCTAL        "o"
#define CMAINLOGIC_NUMSHORTNAME_DECIMAL      "d"
#define CMAINLOGIC_NUMSHORTNAME_HEXADECIMAL  "x"

// operator names
#define CMAINLOGIC_OPNAME_ADD			"add"
#define CMAINLOGIC_OPNAME_SUBTRACT		"subtract"
#define CMAINLOGIC_OPNAME_MULTIPLY		"multiply"
#define CMAINLOGIC_OPNAME_DIVIDE		"divide"
#define CMAINLOGIC_OPNAME_EXPONENTIAL	"exponential"
#define CMAINLOGIC_OPNAME_MODULO		"modulo"
#define CMAINLOGIC_OPNAME_AND			"logical AND"
#define CMAINLOGIC_OPNAME_OR			"logical OR"
#define CMAINLOGIC_OPNAME_XOR			"logical XOR"
#define CMAINLOGIC_OPNAME_INVERT		"invert"
#define CMAINLOGIC_OPNAME_REVERT		"revert"
#define CMAINLOGIC_OPNAME_LSHIFT		"left shift"
#define CMAINLOGIC_OPNAME_RSHIFT		"right shift"
#define CMAINLOGIC_OPNAME_BRACKET_OPEN	"bracket open"
#define CMAINLOGIC_OPNAME_BRACKET_CLOSE	"bracket close"

// operator flags
#define OPFLAG_COMBINE (1 << 0)// combining operators such as +, *, &
#define OPFLAG_LOGICAL (1 << 1)// logical operators such as &, |
#define OPFLAG_MODIFYNUM (1 << 2)// number modifying operators such as +, ~
#define OPFLAG_ALL 0xFFFFFFFFFFFFFFFF// all flags together

// classes
class CSaveFile;

class CMainLogic
{
public:
	// enums
	enum E_NUMTYPES
	{
		NUT_INVALID,// ATTENTION! Invalid must be at first position
		NUT_BINARY,
		NUT_DUAL,
		NUT_OCTAL,
		NUT_DECIMAL,
		NUT_HEXADECIMAL,
		//NUT_ASCII,
		AMOUNT_NUMBERTYPES,
	};

    CMainLogic(bool StartFullscreen, char *pSaveFilePath);
    ~CMainLogic();
    int EntryPoint();
	void RequestApplicationExit();
	int OnConsoleWindowStateChange();

	CLog m_Log;
	CSaveFile *m_pSaveFile;

private:
	// enums
	enum E_COMMANDS
	{
		COM_HELP,
		COM_SET_INPUTFORMAT,
		COM_SET_NUMBERPREFIX,
		COM_SET_OPERATOR,
		COM_SET_RESULTORDER,
		COM_SET_RESPREFIXVIS,
		COM_SET_AUTOSAVE,
		COM_SAVE,
		COM_RESETSETTINGS,
		COM_CLEARSCREEN,
		COM_EXIT,
		AMOUNT_COMMANDS,
	};

	enum E_TOKTYPES
	{
		TOT_NUMBER,
		TOT_OPERATOR,
	};

	enum E_OPTYPES
	{
		OPT_INVALID,// ATTENTION! Invalid must be at first position
		OPT_ADD,
		OPT_SUBTRACT,
		OPT_MULTIPLY,
		OPT_DIVIDE,
		OPT_EXPONENTIAL,
		OPT_MODULO,
		OPT_AND,
		OPT_OR,
		OPT_XOR,
		OPT_INVERT,
		OPT_REVERT,
		OPT_LSHIFT,
		OPT_RSHIFT,
		OPT_BRACKET_OPEN,
		OPT_BRACKET_CLOSE,
		AMOUNT_OPERATORS,
	};

	enum E_USERANSWERS
	{
		ANS_INVALID,
		ANS_YES,
		ANS_NO,
	};

	enum E_COMRETVALS
	{
		CRV_OK,
		CRV_ERROR,
		CRV_INVALIDPARAMS,
		CRV_CANCELLED,
	};

	// structs
	typedef struct
	{
		char m_aInputTokens[CMAINLOGIC_CONSOLE_TOKENS][CMAINLOGIC_CONSOLE_TOKEN_SIZE];
	}S_INPUTTOKENS;

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
		E_TOKTYPES m_TokType;
		E_NUMTYPES m_NumType;
		E_OPTYPES m_OpType;
		U64 m_Number;
		char m_aToken[CMAINLOGIC_CONSOLE_TOKEN_SIZE];
	}S_TOKEN;

	typedef struct
	{
		E_NUMTYPES m_NumType;
		char m_aPrefix[CMAINLOGIC_NUMPREFIXES_LENGTH];
		char m_aName[CMAINLOGIC_NUMNAMES_LENGTH];
		char m_aShortName[CMAINLOGIC_NUMSHORTNAMES_LENGTH];
		int m_Radix;
	}S_NUMBER;

	typedef struct
	{
		E_OPTYPES m_OpType;
		char m_aOperator[CMAINLOGIC_OPERATORS_LENGTH];
		char m_aName[CMAINLOGIC_OPNAMES_LENGTH];
		int m_Flags;
	}S_OPERATOR;

	int ParseInput(const char* pInput, size_t LenInput, S_INPUTTOKENS *psInputTokens);
	int EvaluateTokens(S_INPUTTOKENS *psInputTokens);
	int ExtractNumberFromToken(const char *paToken, U64 *pNumber);
	int ExecuteCommand(S_COMMAND *psCommand, S_INPUTTOKENS *psInputTokens);
	int CheckSyntax(S_TOKEN* pasToken, size_t AmountTokens);
	U64 Calculate(S_TOKEN* pasToken, size_t AmountTokens);
	void PrintResult(U64 Result);
	U64 ModifyNumberByOperator(U64 Number, E_OPTYPES OpType);
	int GetOperatorFlags(E_OPTYPES OpType, int OpFlags);
	bool CheckNumberPrefixCollisions(const char* pPrefix, S_NUMBER** ppsNumberColliding);
	bool CheckOperatorCollisions(const char* pPrefix, S_OPERATOR** ppsOperatorColliding);
	S_OPERATOR* GetOperatorFromType(E_OPTYPES OpType);
	S_OPERATOR* GetOperatorFromType(const char *pType);
	S_OPERATOR* GetOperatorFromToken(const char *paToken);
	S_NUMBER* GetNumberFromType(E_NUMTYPES NumType);
	S_NUMBER* GetNumberFromType(const char *pType);
	S_NUMBER* GetNumberFromPrefix(const char *pPrefix);
	bool CheckStringFormat(const char* pNumber, E_NUMTYPES NumType);
	int NumToString(U64 Number, E_NUMTYPES NumType, char* pResult, size_t LenResult);
	E_USERANSWERS GetUserAnswer(const char *pQuestion, ...);
	int LoadSaveData();
	int SaveSaveData();
	E_COMRETVALS ComHelp(E_COMMANDS ID);
	E_COMRETVALS ComSetinputformat(const char * pType);
	E_COMRETVALS ComSetnumberprefix(const char * pType, const char *pNewPrefix);
	E_COMRETVALS ComSetoperator(const char * pOperator, const char *pNewOperator);
	E_COMRETVALS ComSetresultorder(const char * pOrder);
	E_COMRETVALS ComSetresultprefixvisibility(const char * pOrder);
	E_COMRETVALS ComSetautosave(const char * pOption);
	E_COMRETVALS ComSave(const char * pOption);
	E_COMRETVALS ComResetSettings(const char * pOption);
	E_COMRETVALS ComClearscreen();
	E_COMRETVALS ComExit();

	bool m_StartFullscreen;
	bool m_ExitApplication;
	E_NUMTYPES m_DefaultNumberType;
	bool m_AutoSave;
	char m_aResultOrder[CMAINLOGIC_RESULTORDER_LEN];
	char m_aResultPrefixVis[CMAINLOGIC_RESULTORDER_LEN];
	S_COMMAND m_asCommands[AMOUNT_COMMANDS] = 
	{
		{ COM_HELP,				"help",					"Lists this help",															"",																											"" },
		{ COM_SET_INPUTFORMAT,	"set_inputformat",		"Sets the default number input format (when no prefix is stated)",			"<Number format label (see below \"" CMAINLOGIC_COMHELP_HEADER_NUMPREFIXES_TEXT "\")>",						CMAINLOGIC_NUMNAME_HEXADECIMAL },
		{ COM_SET_NUMBERPREFIX,	"set_numberprefix",		"Changes the prefix of a number format to your preferred string",			"<Number format label (see below \"" CMAINLOGIC_COMHELP_HEADER_NUMPREFIXES_TEXT "\")> <string>",			CMAINLOGIC_NUMNAME_OCTAL " 0w" },
		{ COM_SET_OPERATOR,		"set_operator",			"Changes an operator to your preferred string",								"<Operator label (see below \"" CMAINLOGIC_COMHELP_HEADER_OPERATORS_TEXT "\")> <string>",					CMAINLOGIC_OPNAME_EXPONENTIAL " ^" },
		{ COM_SET_RESULTORDER,	"set_resultorder",		"Sets the order of results (not listed = not visible)",						"<One or more number format short names (see below \"" CMAINLOGIC_COMHELP_HEADER_NUMPREFIXES_TEXT "\")>",	CMAINLOGIC_NUMSHORTNAME_DECIMAL CMAINLOGIC_NUMSHORTNAME_HEXADECIMAL CMAINLOGIC_NUMSHORTNAME_BINARY },
		{ COM_SET_RESPREFIXVIS,	"set_resultprefixvis",	"Sets the visibility of prefixes on results (not listed = not visible)",	"<One or more number format short names (see below \"" CMAINLOGIC_COMHELP_HEADER_NUMPREFIXES_TEXT "\")>",	CMAINLOGIC_NUMSHORTNAME_DECIMAL CMAINLOGIC_NUMSHORTNAME_BINARY },
		{ COM_SET_AUTOSAVE,		"set_autosave",			"Enables or disables auto saving of settings",								"<on/off>",																									"off" },
		{ COM_SAVE,				"save",					"Saves the current settings",												"",																											"" },
		{ COM_RESETSETTINGS,	"reset_settings",		"Resets the settings to their defaults",									"",																											"" },
		{ COM_CLEARSCREEN,		"clear",				"Clears the screen",														"",																											"" },
		{ COM_EXIT,				"exit",					"Exits the program",														"",																											"" },
	};
	S_NUMBER m_asNumbers[AMOUNT_NUMBERTYPES] =
	{
		{ NUT_INVALID,		 "", "invalid",							"",										0},
		{ NUT_BINARY,		 "", CMAINLOGIC_NUMNAME_BINARY,			CMAINLOGIC_NUMSHORTNAME_BINARY,			2 },
		{ NUT_DUAL,			 "", CMAINLOGIC_NUMNAME_DUAL,			CMAINLOGIC_NUMSHORTNAME_DUAL,			4 },
		{ NUT_OCTAL,		 "", CMAINLOGIC_NUMNAME_OCTAL,			CMAINLOGIC_NUMSHORTNAME_OCTAL,			8 },
		{ NUT_DECIMAL,		 "", CMAINLOGIC_NUMNAME_DECIMAL,		CMAINLOGIC_NUMSHORTNAME_DECIMAL,		10 },
		{ NUT_HEXADECIMAL,	 "", CMAINLOGIC_NUMNAME_HEXADECIMAL,	CMAINLOGIC_NUMSHORTNAME_HEXADECIMAL,	16 },
	};
	S_OPERATOR m_asOperators[AMOUNT_OPERATORS] =
	{
		{ OPT_INVALID,			"", "invalid",							0 },
		{ OPT_ADD,				"", CMAINLOGIC_OPNAME_ADD,				OPFLAG_COMBINE | OPFLAG_MODIFYNUM },
		{ OPT_SUBTRACT,			"", CMAINLOGIC_OPNAME_SUBTRACT,			OPFLAG_COMBINE | OPFLAG_MODIFYNUM },
		{ OPT_MULTIPLY,			"", CMAINLOGIC_OPNAME_MULTIPLY,			OPFLAG_COMBINE | OPFLAG_MODIFYNUM },
		{ OPT_DIVIDE,			"", CMAINLOGIC_OPNAME_DIVIDE,			OPFLAG_COMBINE | OPFLAG_MODIFYNUM },
		{ OPT_EXPONENTIAL,		"", CMAINLOGIC_OPNAME_EXPONENTIAL,		OPFLAG_COMBINE | OPFLAG_MODIFYNUM },
		{ OPT_MODULO,			"", CMAINLOGIC_OPNAME_MODULO,			OPFLAG_COMBINE | OPFLAG_MODIFYNUM },
		{ OPT_AND,				"", CMAINLOGIC_OPNAME_AND,				OPFLAG_COMBINE | OPFLAG_LOGICAL },
		{ OPT_OR,				"", CMAINLOGIC_OPNAME_OR,				OPFLAG_COMBINE | OPFLAG_LOGICAL },
		{ OPT_XOR,				"", CMAINLOGIC_OPNAME_XOR,				OPFLAG_COMBINE | OPFLAG_LOGICAL },
		{ OPT_INVERT,			"", CMAINLOGIC_OPNAME_INVERT,			OPFLAG_MODIFYNUM },
		{ OPT_REVERT,			"", CMAINLOGIC_OPNAME_REVERT,			OPFLAG_MODIFYNUM },
		{ OPT_LSHIFT,			"", CMAINLOGIC_OPNAME_LSHIFT,			OPFLAG_COMBINE | OPFLAG_MODIFYNUM },
		{ OPT_RSHIFT,			"", CMAINLOGIC_OPNAME_RSHIFT,			OPFLAG_COMBINE | OPFLAG_MODIFYNUM },
		{ OPT_BRACKET_OPEN,		"", CMAINLOGIC_OPNAME_BRACKET_OPEN,		0 },
		{ OPT_BRACKET_CLOSE,	"", CMAINLOGIC_OPNAME_BRACKET_CLOSE,	0 },
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