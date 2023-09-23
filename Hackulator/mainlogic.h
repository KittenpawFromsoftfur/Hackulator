#pragma once

#include "log.h"
#include "core.h"
#include "savefile.h"

#define DEBUG

// help
#define CMAINLOGIC_MAX_LEN_COMHELP_BUFFERS 256
#define CMAINLOGIC_COMHELP_HEADER_HELP					"<<<<<<<<<<<<<<<<<<<<<<<<< HELP >>>>>>>>>>>>>>>>>>>>>>>>>"
#define CMAINLOGIC_COMHELP_HEADER_COMMANDS				"<<<<<<<<<<<<<<<<<<<<<<< COMMANDS >>>>>>>>>>>>>>>>>>>>>>>"
#define CMAINLOGIC_COMHELP_HEADER_INPPREFIXES_TEXT		"INPUT PREFIXES"
#define CMAINLOGIC_COMHELP_HEADER_INPPREFIXES			"<<<<<<<<<<<<<<<<<<<< " CMAINLOGIC_COMHELP_HEADER_INPPREFIXES_TEXT " >>>>>>>>>>>>>>>>>>>>"
#define CMAINLOGIC_COMHELP_HEADER_OPERATORS_TEXT		"OPERATORS"
#define CMAINLOGIC_COMHELP_HEADER_OPERATORS				"<<<<<<<<<<<<<<<<<<<<<< " CMAINLOGIC_COMHELP_HEADER_OPERATORS_TEXT " >>>>>>>>>>>>>>>>>>>>>>>"
#define CMAINLOGIC_COMHELP_TAILER						"<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
#define CMAINLOGIC_COMHELP_PREFIX						"> "

// inputs and operators
#define CMAINLOGIC_INPPREFIXES_LENGTH 12
#define CMAINLOGIC_INPLABELS_LENGTH 32
#define CMAINLOGIC_INPSHORTLABELS_LENGTH 2
#define CMAINLOGIC_OPERATORS_LENGTH CMAINLOGIC_INPPREFIXES_LENGTH
#define CMAINLOGIC_OPLABELS_LENGTH CMAINLOGIC_INPLABELS_LENGTH
#define CMAINLOGIC_RESULTORDER_LEN 24

// input and tokenization
#define CMAINLOGIC_CONSOLE_BUFFERLEN 256
#define CMAINLOGIC_CONSOLE_TOKENS (CMAINLOGIC_CONSOLE_BUFFERLEN / 2)
#define CMAINLOGIC_CONSOLE_TOKEN_SIZE (64 + CMAINLOGIC_INPPREFIXES_LENGTH)
#define CMAINLOGIC_CONSOLE_USERANSWER_BUFFERLEN 12

// input labels
#define CMAINLOGIC_INPLABEL_BINARY		"binary"
#define CMAINLOGIC_INPLABEL_DUAL		"dual"
#define CMAINLOGIC_INPLABEL_OCTAL		"octal"
#define CMAINLOGIC_INPLABEL_DECIMAL		"decimal"
#define CMAINLOGIC_INPLABEL_HEXADECIMAL	"hexadecimal"
#define CMAINLOGIC_INPLABEL_ASCII		"ascii"

// input short labels
#define CMAINLOGIC_INPSHORTLABEL_BINARY			"b"
#define CMAINLOGIC_INPSHORTLABEL_DUAL			"u"
#define CMAINLOGIC_INPSHORTLABEL_OCTAL			"o"
#define CMAINLOGIC_INPSHORTLABEL_DECIMAL		"d"
#define CMAINLOGIC_INPSHORTLABEL_HEXADECIMAL	"x"
#define CMAINLOGIC_INPSHORTLABEL_ASCII			"a"

// operator labels
#define CMAINLOGIC_OPLABEL_ADD				"add"
#define CMAINLOGIC_OPLABEL_SUBTRACT			"subtract"
#define CMAINLOGIC_OPLABEL_MULTIPLY			"multiply"
#define CMAINLOGIC_OPLABEL_DIVIDE			"divide"
#define CMAINLOGIC_OPLABEL_EXPONENTIAL		"exponential"
#define CMAINLOGIC_OPLABEL_MODULO			"modulo"
#define CMAINLOGIC_OPLABEL_AND				"logical AND"
#define CMAINLOGIC_OPLABEL_OR				"logical OR"
#define CMAINLOGIC_OPLABEL_XOR				"logical XOR"
#define CMAINLOGIC_OPLABEL_INVERT			"invert"
#define CMAINLOGIC_OPLABEL_REVERT			"revert"
#define CMAINLOGIC_OPLABEL_LSHIFT			"left shift"
#define CMAINLOGIC_OPLABEL_RSHIFT			"right shift"
#define CMAINLOGIC_OPLABEL_BRACKET_OPEN		"bracket open"
#define CMAINLOGIC_OPLABEL_BRACKET_CLOSE	"bracket close"

// operator flags
#define OPFLAG_COMBINE (1 << 0)// combining operators such as +, *, &
#define OPFLAG_LOGICAL (1 << 1)// logical operators such as &, |
#define OPFLAG_MODIFYINP (1 << 2)// input modifying operators such as +, ~
#define OPFLAG_ALL 0xFFFFFFFFFFFFFFFF// all flags together

// default values
// input prefixes
#define CMAINLOGIC_DEFAULT_INPPREFIX_BINARY			("0" CMAINLOGIC_INPSHORTLABEL_BINARY)
#define CMAINLOGIC_DEFAULT_INPPREFIX_DUAL			("0" CMAINLOGIC_INPSHORTLABEL_DUAL)
#define CMAINLOGIC_DEFAULT_INPPREFIX_OCTAL			("0" CMAINLOGIC_INPSHORTLABEL_OCTAL)
#define CMAINLOGIC_DEFAULT_INPPREFIX_DECIMAL		("0" CMAINLOGIC_INPSHORTLABEL_DECIMAL)
#define CMAINLOGIC_DEFAULT_INPPREFIX_HEXADECIMAL	("0" CMAINLOGIC_INPSHORTLABEL_HEXADECIMAL)
#define CMAINLOGIC_DEFAULT_INPPREFIX_ASCII			("0" CMAINLOGIC_INPSHORTLABEL_ASCII)

// operators
#define CMAINLOGIC_DEFAULT_OPLABEL_ADD           "+"
#define CMAINLOGIC_DEFAULT_OPLABEL_SUBTRACT      "-"
#define CMAINLOGIC_DEFAULT_OPLABEL_MULTIPLY      "*"
#define CMAINLOGIC_DEFAULT_OPLABEL_DIVIDE        "/"
#define CMAINLOGIC_DEFAULT_OPLABEL_EXPONENTIAL   "\""
#define CMAINLOGIC_DEFAULT_OPLABEL_MODULO        "%"
#define CMAINLOGIC_DEFAULT_OPLABEL_AND           "&"
#define CMAINLOGIC_DEFAULT_OPLABEL_OR            "|"
#define CMAINLOGIC_DEFAULT_OPLABEL_XOR           "^"
#define CMAINLOGIC_DEFAULT_OPLABEL_INVERT        "~"
#define CMAINLOGIC_DEFAULT_OPLABEL_REVERT        "$"
#define CMAINLOGIC_DEFAULT_OPLABEL_LSHIFT        "<<"
#define CMAINLOGIC_DEFAULT_OPLABEL_RSHIFT        ">>"
#define CMAINLOGIC_DEFAULT_OPLABEL_BRACKET_OPEN  "("
#define CMAINLOGIC_DEFAULT_OPLABEL_BRACKET_CLOSE ")"

// other
#define CMAINLOGIC_DEFAULT_RESULTORDER	(CMAINLOGIC_INPSHORTLABEL_BINARY CMAINLOGIC_INPSHORTLABEL_DUAL CMAINLOGIC_INPSHORTLABEL_OCTAL CMAINLOGIC_INPSHORTLABEL_DECIMAL CMAINLOGIC_INPSHORTLABEL_HEXADECIMAL CMAINLOGIC_INPSHORTLABEL_ASCII)
#define CMAINLOGIC_DEFAULT_RESPREFIXVIS (CMAINLOGIC_INPSHORTLABEL_BINARY CMAINLOGIC_INPSHORTLABEL_DUAL CMAINLOGIC_INPSHORTLABEL_OCTAL CMAINLOGIC_INPSHORTLABEL_DECIMAL CMAINLOGIC_INPSHORTLABEL_HEXADECIMAL)
#define CMAINLOGIC_DEFAULT_AUTOSAVE "1"

// classes
class CSaveFile;

class CMainLogic
{
public:
    CMainLogic(bool StartMaximized, char *pSaveFilePath);
    ~CMainLogic();
    int EntryPoint();
	void RequestApplicationExit();

	CLog m_Log;

private:
	// enums
	enum E_COMMANDS
	{
		COM_HELP,
		COM_SET_INPUTFORMAT,
		COM_SET_INPUTPREFIX,
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
		TOT_INVALID,
		TOT_INPUT,
		TOT_OPERATOR,
	};

	enum E_INPTYPES
	{
		INT_INVALID,// ATTENTION! Invalid must be at first position
		INT_BINARY,
		INT_DUAL,
		INT_OCTAL,
		INT_DECIMAL,
		INT_HEXADECIMAL,
		INT_ASCII,
		AMOUNT_INPTYPES,
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
		AMOUNT_OPTYPES,
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
		E_COMMANDS m_ID;
		char m_aLabel[CMAINLOGIC_CONSOLE_TOKEN_SIZE];
		char m_aDescription[CMAINLOGIC_MAX_LEN_COMHELP_BUFFERS];
		char m_aParameters[CMAINLOGIC_MAX_LEN_COMHELP_BUFFERS];
		char m_aExample[CMAINLOGIC_MAX_LEN_COMHELP_BUFFERS];
	} S_COMMAND;

	typedef struct
	{
		E_TOKTYPES m_TokType;
		E_INPTYPES m_InpType;
		E_OPTYPES m_OpType;
		U64 m_Number;
		char m_aToken[CMAINLOGIC_CONSOLE_TOKEN_SIZE];
	}S_TOKEN;

	typedef struct
	{
		E_INPTYPES m_InpType;
		char m_aPrefix[CMAINLOGIC_INPPREFIXES_LENGTH];
		char m_aLabel[CMAINLOGIC_INPLABELS_LENGTH];
		char m_aShortLabel[CMAINLOGIC_INPSHORTLABELS_LENGTH];
		int m_Radix;
	}S_INPUT;

	typedef struct
	{
		E_OPTYPES m_OpType;
		char m_aOperator[CMAINLOGIC_OPERATORS_LENGTH];
		char m_aLabel[CMAINLOGIC_OPLABELS_LENGTH];
		int m_Flags;
	}S_OPERATOR;

	typedef struct
	{
		char m_aaInputTokens[CMAINLOGIC_CONSOLE_TOKENS][CMAINLOGIC_CONSOLE_TOKEN_SIZE];
	}S_INPUTTOKENS;

	int TokenizeInput(const char* pInput, size_t LenInput, S_INPUTTOKENS *psInputTokens);
	int EvaluateTokens(S_INPUTTOKENS *psInputTokens);
	int ConvertInputToToken(const char *pToken, S_TOKEN *psToken);
	S_INPUT* CopyInputWithoutPrefix(const char* pToken, char* pContent, size_t LenContent, bool *pWasPrefixed);
	int ExecuteCommand(S_COMMAND *psCommand, S_INPUTTOKENS *psInputTokens);
	int CheckSyntax(S_TOKEN* pasToken, size_t AmountTokens);
	U64 Calculate(S_TOKEN* pasToken, size_t AmountTokens);
	void PrintResult(U64 Result);
	U64 ModifyInputByOperator(U64 Number, E_OPTYPES OpType);
	int GetOperatorFlags(E_OPTYPES OpType, int OpFlags);
	bool CheckInputPrefixCollisions(const char* pPrefix, S_INPUT** ppsInputColliding);
	bool CheckOperatorCollisions(const char* pPrefix, S_OPERATOR** ppsOperatorColliding);
	S_OPERATOR* GetOperatorFromType(E_OPTYPES OpType);
	S_OPERATOR* GetOperatorFromLabel(const char *pLabel);
	S_OPERATOR* GetOperatorFromString(const char *pString);
	S_INPUT* GetInputFromType(E_INPTYPES InpType);
	S_INPUT* GetInputFromLabel(const char *pLabel);
	S_INPUT* GetInputFromString(const char *pString);
	bool CheckInputFormat(const char* pInput, E_INPTYPES InpType);
	int InputToString(U64 Number, E_INPTYPES InpType, char* pResult, size_t LenResult);
	E_USERANSWERS GetUserAnswer(const char *pQuestion, ...);
	int LoadSaveData();
	int SaveSaveData();
	E_COMRETVALS ComHelp(E_COMMANDS ID);
	E_COMRETVALS ComSetinputformat(const char * pType);
	E_COMRETVALS ComSetinputprefix(const char * pType, const char *pNewPrefix);
	E_COMRETVALS ComSetoperator(const char * pOperator, const char *pNewOperator);
	E_COMRETVALS ComSetresultorder(const char * pOrder);
	E_COMRETVALS ComSetresultprefixvisibility(const char * pOrder);
	E_COMRETVALS ComSetautosave(const char * pOption);
	E_COMRETVALS ComSave(const char * pOption);
	E_COMRETVALS ComResetSettings(const char * pOption);
	E_COMRETVALS ComClearscreen();
	E_COMRETVALS ComExit();

	bool m_StartMaximized;
	bool m_ExitApplication;
	E_INPTYPES m_DefaultInputType;
	bool m_AutoSave;
	char m_aResultOrder[CMAINLOGIC_RESULTORDER_LEN];
	char m_aResultPrefixVis[CMAINLOGIC_RESULTORDER_LEN];
	CSaveFile m_SaveFile;
	S_COMMAND m_asCommands[AMOUNT_COMMANDS] = 
	{
		{ COM_HELP,				"help",					"Lists this help",															"",																											"" },
		{ COM_SET_INPUTFORMAT,	"set_inputformat",		"Sets the default input format (when no prefix is stated)",					"<Input format label (see below \"" CMAINLOGIC_COMHELP_HEADER_INPPREFIXES_TEXT "\")>",						CMAINLOGIC_INPLABEL_HEXADECIMAL },
		{ COM_SET_INPUTPREFIX,	"set_inputprefix",		"Changes the prefix of an input format to your preferred string",			"<Input format label (see below \"" CMAINLOGIC_COMHELP_HEADER_INPPREFIXES_TEXT "\")> <string>",				CMAINLOGIC_INPLABEL_OCTAL " 0w" },
		{ COM_SET_OPERATOR,		"set_operator",			"Changes an operator to your preferred string",								"<Operator label (see below \"" CMAINLOGIC_COMHELP_HEADER_OPERATORS_TEXT "\")> <string>",					CMAINLOGIC_OPLABEL_EXPONENTIAL " ^" },
		{ COM_SET_RESULTORDER,	"set_resultorder",		"Sets the order of results (not listed = not visible)",						"<One or more input format short names (see below \"" CMAINLOGIC_COMHELP_HEADER_INPPREFIXES_TEXT "\")>",	CMAINLOGIC_INPSHORTLABEL_DECIMAL CMAINLOGIC_INPSHORTLABEL_HEXADECIMAL CMAINLOGIC_INPSHORTLABEL_BINARY },
		{ COM_SET_RESPREFIXVIS,	"set_resultprefixvis",	"Sets the visibility of prefixes on results (not listed = not visible)",	"<One or more input format short names (see below \"" CMAINLOGIC_COMHELP_HEADER_INPPREFIXES_TEXT "\")>",	CMAINLOGIC_INPSHORTLABEL_DECIMAL CMAINLOGIC_INPSHORTLABEL_BINARY },
		{ COM_SET_AUTOSAVE,		"set_autosave",			"Enables or disables auto saving of settings",								"<on/off>",																									"off" },
		{ COM_SAVE,				"save",					"Saves the current settings",												"",																											"" },
		{ COM_RESETSETTINGS,	"reset_settings",		"Resets the settings to their defaults",									"",																											"" },
		{ COM_CLEARSCREEN,		"clear",				"Clears the screen",														"",																											"" },
		{ COM_EXIT,				"exit",					"Exits the program",														"",																											"" },
	};
	S_INPUT m_asInputs[AMOUNT_INPTYPES] =
	{
		{ INT_INVALID,		"", "invalid",							"",										0},
		{ INT_BINARY,		"", CMAINLOGIC_INPLABEL_BINARY,			CMAINLOGIC_INPSHORTLABEL_BINARY,		2 },
		{ INT_DUAL,			"", CMAINLOGIC_INPLABEL_DUAL,			CMAINLOGIC_INPSHORTLABEL_DUAL,			4 },
		{ INT_OCTAL,		"", CMAINLOGIC_INPLABEL_OCTAL,			CMAINLOGIC_INPSHORTLABEL_OCTAL,			8 },
		{ INT_DECIMAL,		"", CMAINLOGIC_INPLABEL_DECIMAL,		CMAINLOGIC_INPSHORTLABEL_DECIMAL,		10 },
		{ INT_HEXADECIMAL,	"", CMAINLOGIC_INPLABEL_HEXADECIMAL,	CMAINLOGIC_INPSHORTLABEL_HEXADECIMAL,	16 },
		{ INT_ASCII,		"", CMAINLOGIC_INPLABEL_ASCII,			CMAINLOGIC_INPSHORTLABEL_ASCII,			0 },
	};
	S_OPERATOR m_asOperators[AMOUNT_OPTYPES] =
	{
		{ OPT_INVALID,			"", "invalid",							0 },
		{ OPT_ADD,				"", CMAINLOGIC_OPLABEL_ADD,				OPFLAG_COMBINE | OPFLAG_MODIFYINP },
		{ OPT_SUBTRACT,			"", CMAINLOGIC_OPLABEL_SUBTRACT,		OPFLAG_COMBINE | OPFLAG_MODIFYINP },
		{ OPT_MULTIPLY,			"", CMAINLOGIC_OPLABEL_MULTIPLY,		OPFLAG_COMBINE | OPFLAG_MODIFYINP },
		{ OPT_DIVIDE,			"", CMAINLOGIC_OPLABEL_DIVIDE,			OPFLAG_COMBINE | OPFLAG_MODIFYINP },
		{ OPT_EXPONENTIAL,		"", CMAINLOGIC_OPLABEL_EXPONENTIAL,		OPFLAG_COMBINE | OPFLAG_MODIFYINP },
		{ OPT_MODULO,			"", CMAINLOGIC_OPLABEL_MODULO,			OPFLAG_COMBINE | OPFLAG_MODIFYINP },
		{ OPT_AND,				"", CMAINLOGIC_OPLABEL_AND,				OPFLAG_COMBINE | OPFLAG_LOGICAL },
		{ OPT_OR,				"", CMAINLOGIC_OPLABEL_OR,				OPFLAG_COMBINE | OPFLAG_LOGICAL },
		{ OPT_XOR,				"", CMAINLOGIC_OPLABEL_XOR,				OPFLAG_COMBINE | OPFLAG_LOGICAL },
		{ OPT_INVERT,			"", CMAINLOGIC_OPLABEL_INVERT,			OPFLAG_MODIFYINP },
		{ OPT_REVERT,			"", CMAINLOGIC_OPLABEL_REVERT,			OPFLAG_MODIFYINP },
		{ OPT_LSHIFT,			"", CMAINLOGIC_OPLABEL_LSHIFT,			OPFLAG_COMBINE | OPFLAG_MODIFYINP },
		{ OPT_RSHIFT,			"", CMAINLOGIC_OPLABEL_RSHIFT,			OPFLAG_COMBINE | OPFLAG_MODIFYINP },
		{ OPT_BRACKET_OPEN,		"", CMAINLOGIC_OPLABEL_BRACKET_OPEN,	0 },
		{ OPT_BRACKET_CLOSE,	"", CMAINLOGIC_OPLABEL_BRACKET_CLOSE,	0 },
	};
	char m_aASCIIDescriptions[34][32] =
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
		"DEL (delete)",
	};
};