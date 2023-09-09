#include "mainlogic.h"

/* TODO
	m_aNumberTypeNames to struct same as S_SIGN, so you dont have to use constructor...
	square ("), <<, >>, %
	#<ascii value>
	custom prefixes and symbols
	tabulator, space as allowed whitespaces
	aaToken[][] --> *
	help should explain every single aspect of the hackulator
	Use math library to not have to make own syntax
		ExprEval: https://expreval.sourceforge.net/#:~:text=ExprEval%20is%20a%20powerful%2C%20high,once%20into%20an%20expression%20tree.
		TinyExpr: https://github.com/codeplea/tinyexpr
	Algorithms to code self: "Shunting yard", "Reverse polish notation"
	Save file; Help output save file location --> OR <-- main parameters, so you can create a shortcut and it acts as a save file
*/

int main()
{
	int retval = 0;
	CMainLogic mainLogic;

	retval = mainLogic.EntryPoint();

	return retval;
}




































/*
	|, &, ^, ~, <<, >>
*/

/*********************************************************************************
* @brief Entry point of the application
*
* @retval 0
*********************************************************************************/
/*int main_old()
{
	int i = 0;
	int returnValue = 0;
	int commandEntered = FALSE;
	char aInput[256] = { 0 };
	unsigned long long aInputValues[ARRAY_SIZE(aInput) / 2] = { 0 };
	E_OPERATOR_TYPES aOperatorTypes[ARRAY_SIZE(aInputValues)] = { 0 };
	unsigned long long sum = 0;

	while (1)
	{
		// resets
		memset(aInput, 0, ARRAY_SIZE(aInput));

		for (i = 0; i < ARRAY_SIZE(aInputValues); ++i)
			aInputValues[i] = 0;

		for (i = 0; i < ARRAY_SIZE(aOperatorTypes); ++i)
			aOperatorTypes[i] = 0;

		// get user input
		returnValue = scanf(" %100[^\n]", aInput);

		if (returnValue <= 0)
		{
			printf("%s: Error scanning input\n", __FUNCTION__);
			continue;
		}

		// evaluate command and call associated function
		commandEntered = FALSE;

		for (i = 0; i < ARRAY_SIZE(gasConsoleCommands); ++i)
		{
			if (strncmp_nocase(aInput, gasConsoleCommands[i].aString, ARRAY_SIZE(gasConsoleCommands[0].aString)) == 0)
			{
				commandEntered = TRUE;
				gasConsoleCommands[i].pFunction(FALSE);
				break;
			}
		}

		// evaluate input
		if (!commandEntered)
		{
			bitconv_filterSpaces(aInput);

			returnValue = bitconv_parseChainedInput(aInput, aInputValues, aOperatorTypes, ARRAY_SIZE(aInputValues));

			// bubble sort values for subtractions
			for (int i = 0; i < ARRAY_SIZE(aInputValues) - 1; i++)
			{
				// Last i elements are already in place
				for (int j = 0; j < ARRAY_SIZE(aInputValues) - i - 1; j++)
					if (aInputValues[j] < aInputValues[j + 1])
						swapUll(&aInputValues[j], &aInputValues[j + 1]);
			}

			sum = aInputValues[0];

			for (i = 1; i < returnValue; ++i)
			{
				if (aOperatorTypes[i - 1] == E_OPTYPE_ADD)
					sum += aInputValues[i];
				else
					sum -= aInputValues[i];
			}

			bitconv_showInputResult(sum);
		}
	}

	return 0;
}*/