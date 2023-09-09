#pragma once

#define BITCONVERTER_MAX_RANGE_ALLOWED "64"

#define PREFIX_DECIMAL ".d"
#define PREFIX_BINARY ".b"
#define PREFIX_ASCII "#"

#define OPERATOR_ADD "+"
#define OPERATOR_SUBTRACT "-"
#define OPERATOR_BITWISE_OR "|"
#define OPERATOR_BITWISE_AND "&"
#define OPERATOR_BITWISE_XOR "^"
#define OPERATOR_BITWISE_INVERT "~"
#define OPERATOR_BITWISE_LSHIFT "<<"
#define OPERATOR_BITWISE_RSHIFT ">>"

// enums
typedef enum
{
	E_IPTYPE_DECIMAL = 0,
	E_IPTYPE_BINARY,
	E_IPTYPE_ASCII,
}E_INPUT_TYPES;

typedef enum
{
	E_OPTYPE_ADD = 0,
	E_OPTYPE_SUBTRACT,
	E_OPTYPE_BITWISE_OR,
	E_OPTYPE_BITWISE_AND,
	E_OPTYPE_BITWISE_XOR,
	E_OPTYPE_BITWISE_INVERT,
	E_OPTYPE_BITWISE_LSHIFT,
	E_OPTYPE_BITWISE_RSHIFT,
}E_OPERATOR_TYPES;

typedef enum
{
	E_COMMAND_HELP = 0,
	E_COMMAND_CLEARSCREEN,
	E_COMMAND_QUIT,
}E_COMMANDS;

// structs
typedef struct
{
	char aString[256];
	int value;
}S_STRING_VALUE_PAIR;

typedef struct
{
	char aString[256];
	void (*pFunction)();
}S_STRING_FUNCTION_PAIR;

// prototypes
int bitconv_parseChainedInput(char *pInput, unsigned long long *pValues, E_OPERATOR_TYPES *pCalcTypes, int MaxValues);
unsigned long long bitconv_inputToNumber(char *pInput, int Size);
void bitconv_showInputResult(unsigned long long Number);
void bitconv_decimalToBinary(unsigned long long Number, char *pResult, int Size);
unsigned long long bitconv_binaryToDecimal(char *pBinary, int Size);
unsigned long long bitconv_hexToDecimal(char *pHex, int Size);
void bitconv_filterSpaces(char *pInput);
static inline void bitconv_comShowHelp(int ShowHelp);
static inline void bitconv_comClearScreen(int ShowHelp);
static inline void bitconv_comQuit(int ShowHelp);

extern S_STRING_VALUE_PAIR gasHexBaseValues[22];
extern S_STRING_FUNCTION_PAIR gasConsoleCommands[9];
extern S_STRING_VALUE_PAIR gasPrefixIptypePair[3];
extern S_STRING_VALUE_PAIR gasOperatorOptypePair[2];
extern char gaAsciiSigns1[][32];
extern char gaAsciiSigns2[][32];