#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <Windows.h>

#include "functions.h"
#include "bitconverter.h"

// global variables
S_STRING_VALUE_PAIR gasHexBaseValues[] =
{
    { "0", 0 },
    { "1", 1 },
    { "2", 2 },
    { "3", 3 },
    { "4", 4 },
    { "5", 5 },
    { "6", 6 },
    { "7", 7 },
    { "8", 8 },
    { "9", 9 },
    { "a", 10 },
    { "b", 11 },
    { "c", 12 },
    { "d", 13 },
    { "e", 14 },
    { "f", 15 },
};

S_STRING_FUNCTION_PAIR gasConsoleCommands[] =
{
    { "help", bitconv_comShowHelp },
    { "?", bitconv_comShowHelp },
    { "clear", bitconv_comClearScreen },
    { "cls", bitconv_comClearScreen },
    { "quit", bitconv_comQuit },
    { "q", bitconv_comQuit },
    { "exit", bitconv_comQuit },
    { "close", bitconv_comQuit },
};

S_STRING_VALUE_PAIR gasPrefixIptypePair[] =
{
    { PREFIX_DECIMAL, E_IPTYPE_DECIMAL },
    { PREFIX_BINARY, E_IPTYPE_BINARY },
    { PREFIX_ASCII, E_IPTYPE_ASCII },
};

S_STRING_VALUE_PAIR gasOperatorOptypePair[] =
{
    { OPERATOR_ADD, E_OPTYPE_ADD },
    { OPERATOR_SUBTRACT, E_OPTYPE_SUBTRACT },
    /*{ OPERATOR_BITWISE_OR, E_OPTYPE_BITWISE_OR },
    { OPERATOR_BITWISE_AND, E_OPTYPE_BITWISE_AND },
    { OPERATOR_BITWISE_XOR, E_OPTYPE_BITWISE_XOR },
    { OPERATOR_BITWISE_INVERT, E_OPTYPE_BITWISE_INVERT },
    { OPERATOR_BITWISE_LSHIFT, E_OPTYPE_BITWISE_LSHIFT },
    { OPERATOR_BITWISE_RSHIFT, E_OPTYPE_BITWISE_RSHIFT },*/
};

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

/*********************************************************************************
* @brief Parses the input line and separates them by operators, then returns the
*        calculated values and operator types in an array
*
* @param[in]  *pInput     String
* @param[out] *pValues    Pointer to array with values
* @param[out] *pCalcTypes Pointer to array with operator types
* @param[in]  MaxValues   Max amount of values to store into arrays
*
* @retval The amount of values that were parsed
*********************************************************************************/
int bitconv_parseChainedInput(char *pInput, unsigned long long *pValues, E_OPERATOR_TYPES *pOperatorTypes, int MaxValues)
{
    int i = 0;
    int ii = 0;
    int pos = 0;
    int amountValues = 0;
    E_OPERATOR_TYPES operatorType = -1;
    const int maxLenInput = strlen(pInput) + 1;
    char *aTemp = (char *)malloc(maxLenInput);

    if (aTemp == NULL)
        return 0;

    memset(aTemp, 0, maxLenInput);

    for (i = 0; i < maxLenInput; ++i)
    {
        operatorType = -1;

        // check for operators
        for (ii = 0; ii < ARRAY_SIZE(gasOperatorOptypePair); ++ii)
        {
            if (pInput[i] == gasOperatorOptypePair[ii].aString[0])
            {
                operatorType = gasOperatorOptypePair[ii].value;
                break;
            }
        }

        // operator or end of input found
        if (operatorType != -1 || pInput[i] == 0)
        {
            amountValues++;

            pValues[amountValues - 1] = bitconv_inputToNumber(aTemp, maxLenInput);
            pOperatorTypes[amountValues - 1] = operatorType;

            pos = 0;
            memset(aTemp, 0, maxLenInput);

            if (amountValues >= MaxValues)
                break;
        }
        else// copy input to temp
        {
            aTemp[pos] = pInput[i];
            pos++;
        }

        if (pInput[i] == 0)
            break;
    }

    free(aTemp);

    return amountValues;
}

/*********************************************************************************
* @brief Converts an input to a number
*
* @param[in] *pInput String
* @param[in] Size    Max length of input
*
* @retval Converted number
*********************************************************************************/
unsigned long long bitconv_inputToNumber(char *pInput, int Size)
{
    int i = 0;
    int prefixStrLen = 0;
    unsigned long long number = 0;
    E_INPUT_TYPES inputType = -1;

    // evaluate prefix type
    for (i = 0; i < ARRAY_SIZE(gasPrefixIptypePair); ++i)
    {
        if (strncmp_nocase(pInput, gasPrefixIptypePair[i].aString, MIN(strlen(gasPrefixIptypePair[i].aString), Size)) == 0)
        {
            inputType = gasPrefixIptypePair[i].value;
            break;
        }
    }

    // remove prefix
    if (inputType != -1)
    {
        prefixStrLen = strlen(gasPrefixIptypePair[inputType].aString);

        for (i = 0; i < MAX(0, Size - prefixStrLen); ++i)
            pInput[i] = pInput[i + prefixStrLen];
    }

    // transform input into number
    switch (inputType)
    {
    case E_IPTYPE_DECIMAL:
        number = strtoull(pInput, NULL, 10);
        break;

    case E_IPTYPE_BINARY:
        number = bitconv_binaryToDecimal(pInput, Size);
        break;

    case E_IPTYPE_ASCII:
        if (strlen(pInput) == 1)// ascii can only be one character
            number = (unsigned char)pInput[0];
        else
            number = 0;
        break;

    default:// hexadecimal
        number = bitconv_hexToDecimal(pInput, Size);
        break;
    }

    return number;
}

/*********************************************************************************
* @brief Outputs a number in multiple numbering systems
*
* @param[in] Number Number
*********************************************************************************/
void bitconv_showInputResult(unsigned long long Number)
{
    char aNumberBinary[256] = { 0 };
    char aAsciiRep[256] = { 0 };

    bitconv_decimalToBinary(Number, aNumberBinary, ARRAY_SIZE(aNumberBinary));

    // ascii representation
    if (Number < ARRAY_SIZE(gaAsciiSigns1) && Number < 127)
    {
        strncpy(aAsciiRep, gaAsciiSigns1[Number], ARRAY_SIZE(aAsciiRep));
    }
    else if (Number < 127)
    {
        snprintf(aAsciiRep, ARRAY_SIZE(aAsciiRep), "%c", Number);
    }
    else if (Number < 128)
    {
        strncpy(aAsciiRep, gaAsciiSigns2[Number - 127], ARRAY_SIZE(aAsciiRep));
    }

    // result
    printf("%llu | 0x%llX | 0b%s | %s\n", Number, Number, aNumberBinary, aAsciiRep);
}

/*********************************************************************************
* @brief Converts a decimal number into a binary number
*
* @param[in]  Number   Number to be converted
* @param[out] *pResult Pointer to array where the result is stored
* @param[in]  Size     Max size of the array
*********************************************************************************/
void bitconv_decimalToBinary(unsigned long long Number, char *pResult, int Size)
{
    int i = 0;
    char sign = 0;
    int lastPosOne = 0;

    memset(pResult, 0, Size);

    for (i = 0; i < MIN(sizeof(unsigned long long) * 8, Size); i++)
    {
        sign = (((Number & (1LLU << i)) > 0) ? '1' : '0');

        if (sign != '0')
            lastPosOne = i;

        pResult[i] = sign;
    }

    pResult[lastPosOne + 1] = '\0';

    revertString(pResult);
}

/*********************************************************************************
* @brief Converts an binary number to a decimal number
*
* @param[in] *pBinary String in binary format
* @param[in] Size     Max length of string
*
* @retval Converted number
*********************************************************************************/
unsigned long long bitconv_binaryToDecimal(char *pBinary, int Size)
{
    int i = 0;
    unsigned long long value = 0;
    int stringLen = 0;

    stringLen = MIN(strlen(pBinary), Size);

    revertString(pBinary);

    if (!strIsBinaryNumber(pBinary))
    {
        return 0;
    }

    for (i = 0; i < stringLen; ++i)
    {
        value += ((pBinary[i] == '1') ? (1LLU << i) : 0);
    }

    return value;
}

/*********************************************************************************
* @brief Converts an hex number to a decimal number
*
* @param[in] *pHex String in hex format
* @param[in] Size  Max length of string
*
* @retval Converted number
*********************************************************************************/
unsigned long long bitconv_hexToDecimal(char *pHex, int Size)
{
    int i = 0, ii = 0;
    unsigned long long value = 0;
    int stringLen = 0;

    stringLen = MIN(strlen(pHex), Size);

    revertString(pHex);

    if (!strIsHexNumber(pHex))
    {
        return 0;
    }

    for (i = 0; i < stringLen; ++i)
    {
        for (ii = 0; ii < ARRAY_SIZE(gasHexBaseValues); ++ii)
        {
            if (gasHexBaseValues[ii].aString[0] == tolower(pHex[i]))
            {
                value += gasHexBaseValues[ii].value * powull(16, i);
                break;
            }
        }
    }

    return value;
}

/*********************************************************************************
* @brief Filters spaces out of a string, considering that spaces can not be
*        filtered out in all circumstances
*
* @param[out] *pString     Input string
*********************************************************************************/
void bitconv_filterSpaces(char *pInput)
{
    int i = 0;
    int ii = 0;
    int pos = 0;

    for (i = 0; i < strlen(pInput); ++i)
    {
        if (pInput[i] != ' ' || strncmp(pInput + i * sizeof(char) - strlen(PREFIX_ASCII) * sizeof(char), PREFIX_ASCII, strlen(PREFIX_ASCII)) == 0)
        {
            pInput[pos] = pInput[i];
            pos++;
        }
    }

    pInput[pos] = '\0';
}

/*********************************************************************************
* @brief Displays help for the program
*********************************************************************************/
static inline void bitconv_comShowHelp(int ShowHelp)
{
    if (ShowHelp)
    {
        printf("Shows this help\n");
        return;
    }

    printf
    (
        "<<< Info >>>\n"
        "All input is case insensitive and spaces are filtered\n"
        "Subtracting numbers will always display positive result\n"
        "Max range is " BITCONVERTER_MAX_RANGE_ALLOWED " bit\n"
        "\n"
        "<<< Commands >>>\n"
    );

    // count amount of different commands
    int amtComms = 0;
    int i = 0;
    int ii = 0;
    long long aCommFuncTemp[ARRAY_SIZE(gasConsoleCommands)] = { 0 };
    int aCommsAmt[ARRAY_SIZE(aCommFuncTemp)] = { 0 };
    int foundOccurrence = FALSE;

    for (i = 0; i < ARRAY_SIZE(gasConsoleCommands); ++i)
    {
        foundOccurrence = FALSE;

        // see if an occurrence of the function already exists in temp
        for (ii = 0; ii < ARRAY_SIZE(aCommFuncTemp); ++ii)
        {
            if (aCommFuncTemp[ii] == gasConsoleCommands[i].pFunction)
            {
                foundOccurrence = TRUE;
                break;
            }
        }

        // add first function occurrences to temp
        if (!foundOccurrence)
        {
            aCommFuncTemp[amtComms] = gasConsoleCommands[i].pFunction;
            amtComms++;
        }
    }

    // we now have all command functions that occur at least once in the whole command list
    // find out how often each function is assigned
    for (i = 0; i < ARRAY_SIZE(aCommFuncTemp); ++i)
    {
        for (ii = 0; ii < ARRAY_SIZE(gasConsoleCommands); ++ii)
        {
            if (aCommFuncTemp[i] == gasConsoleCommands[ii].pFunction)
            {
                aCommsAmt[i]++;
            }
        }
    }

    // create collected commands and description
    int aCommsAmtUsed[ARRAY_SIZE(aCommsAmt)] = { 0 };
    char aCommsCollected[ARRAY_SIZE(aCommFuncTemp)][ARRAY_SIZE(gasConsoleCommands[0].aString)] = { 0 };
    char aTemp[ARRAY_SIZE(gasConsoleCommands[0].aString)] = { 0 };
    char aSeparator[5] = { 0 };

    for (i = 0; i < ARRAY_SIZE(aCommFuncTemp); ++i)
    {
        for (ii = 0; ii < ARRAY_SIZE(gasConsoleCommands); ++ii)
        {
            if (aCommFuncTemp[i] == gasConsoleCommands[ii].pFunction)
            {
                // change separtor
                if (strlen(aCommsCollected[i]) == 0)
                    strncpy(aSeparator, "", ARRAY_SIZE(aSeparator));
                else if (aCommsAmtUsed[i] < aCommsAmt[i] - 1)
                    strncpy(aSeparator, ", ", ARRAY_SIZE(aSeparator));
                else
                    strncpy(aSeparator, " or ", ARRAY_SIZE(aSeparator));

                memset(aTemp, 0, ARRAY_SIZE(aTemp));
                snprintf(aTemp, ARRAY_SIZE(aTemp), "%s%s'%s'", aCommsCollected[i], aSeparator, gasConsoleCommands[ii].aString);
                strncpy(aCommsCollected[i], aTemp, ARRAY_SIZE(aCommsCollected[i]));

                aCommsAmtUsed[i]++;
            }
        }
    }

    // Output the collected commands
    for (i = 0; i < ARRAY_SIZE(aCommsCollected); ++i)
    {
        if (aCommFuncTemp[i] == 0)
            break;

        printf("%s... ", aCommsCollected[i]);

        // call associated function once
        for (ii = 0; ii < ARRAY_SIZE(gasConsoleCommands); ++ii)
        {
            if (aCommFuncTemp[i] == gasConsoleCommands[ii].pFunction)
            {
                gasConsoleCommands[ii].pFunction(TRUE);
                break;
            }
        }
    }

    printf
    (
        "\n"
        "<<< Usage >>>\n"
        "Enter a number in either hexadecimal (<input>), decimal (" PREFIX_DECIMAL "<input>), binary (" PREFIX_BINARY "<input>) or ascii (" PREFIX_ASCII "<character>)\n"
        "You can add (" OPERATOR_ADD ") and subtract (" OPERATOR_SUBTRACT ") multiple inputs\n"
        //"You can use the following bitwise operators on two inputs: " OPERATOR_BITWISE_AND ", " OPERATOR_BITWISE_OR ", " OPERATOR_BITWISE_INVERT ", " OPERATOR_BITWISE_XOR ", " OPERATOR_BITWISE_LSHIFT ", " OPERATOR_BITWISE_RSHIFT "\n"
        "\n"
    );
}

/*********************************************************************************
* @brief Clears the screen of the console
*********************************************************************************/
static inline void bitconv_comClearScreen(int ShowHelp)
{
    if (ShowHelp)
    {
        printf("Clears the screen\n");
        return;
    }

    system("cls");
}

/*********************************************************************************
* @brief Exits the program
*********************************************************************************/
static inline void bitconv_comQuit(int ShowHelp)
{
    if (ShowHelp)
    {
        printf("Ends the program\n");
        return;
    }

    exit(0);
}