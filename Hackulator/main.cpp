#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "mainlogic.h"

/* TODO
	struct members: m_...
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

int main(int argc, char *argv[])
{
	char* pParam = 0;
	char* pParamValue = 0;
	bool failed = false;
	bool paramStartfullscreen = false;

	// parse call parameters
	for (int i = 1; i < argc; ++i)
	{
		// determine param
		pParam = argv[i];

		// determine param value
		pParamValue = 0;

		if (argc > i + 1)
			pParamValue = argv[i + 1];

		// compare params
		if (strncmp(pParam, MAINPARAM_STARTFULLSCREEN, MAX_LEN_MAINPARAMS) == 0)
		{
			if (pParamValue)
			{
				paramStartfullscreen = atoi(pParamValue);
				i++;
			}
			else
			{
				failed = true;
			}
		}
		else
		{
			failed = true;
		}

		if (failed)
			break;
	}

	if (failed)
	{
		printf("\nInvalid parameters, usage:");
		printf("\n\n" MAINPARAMHELP_HEADER);
		printf("\n%s... %s", MAINPARAM_STARTFULLSCREEN, MAINPARAM_STARTFULLSCREEN_DESC);
		printf("\n" MAINPARAMHELP_HEADER "\n\n");
		return ERROR;
	}

	CMainLogic mainLogic(paramStartfullscreen);

	return mainLogic.EntryPoint();
}