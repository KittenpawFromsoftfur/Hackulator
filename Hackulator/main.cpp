#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

#include "main.h"
#include "mainlogic.h"

/* TODO
	INT_ASCII
	PrintReslut() more elegant with arrays
	Operator and Number inheritance from same base class
	savefile has Init() and all default values are changed by function from mainlogic constructor --> private E_INPTYPES again
	usable as command line tool
	help should explain every single aspect of the hackulator (help file cause too complicated? generatehelp?)
	Use math library to not have to make own syntax (Otherwise Algorithms to code self: "Shunting yard", "Reverse polish notation")
		ExprEval: https://expreval.sourceforge.net/#:~:text=ExprEval%20is%20a%20powerful%2C%20high,once%20into%20an%20expression%20tree.
		TinyExpr: https://github.com/codeplea/tinyexpr
*/

int main(int argc, char *argv[])
{
	char* pParam = 0;
	char* pParamValue = 0;
	bool failed = false;
	bool paramStartmaximized = false;
	char aSaveFilePath[MAX_LEN_FILEPATHS] = { 0 };
	char aProgPathCropped[ARRAYSIZE(aSaveFilePath)] = { 0 };
	char* pProgPathLastSlash = 0;

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
		if (strncmp(pParam, MAIN_PARAM_STARTMAXIMIZED, MAIN_MAX_LEN_PARAMS) == 0)
		{
			if (pParamValue)
			{
				paramStartmaximized = atoi(pParamValue);
				i++;
			}
			else
			{
				failed = true;
			}
		}
		else if (strncmp(pParam, MAIN_PARAM_SAVEFILEPATH, MAIN_MAX_LEN_PARAMS) == 0)
		{
			if (pParamValue)
			{
				strncpy_s(aSaveFilePath, pParamValue, ARRAYSIZE(aSaveFilePath));
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

	// output help if parameters are wrong
	if (failed)
	{
		printf("\nInvalid parameters, usage:");
		printf("\n\n" MAIN_PARAMHELP_HEADER);
		printf("\n%s... %s", MAIN_PARAM_STARTMAXIMIZED, MAIN_PARAM_STARTMAXIMIZED_DESC);
		printf("\n%s... %s", MAIN_PARAM_SAVEFILEPATH, MAIN_PARAM_SAVEFILEPATH_DESC);
		printf("\n" MAIN_PARAMHELP_HEADER "\n\n");
		return ERROR;
	}

	// determine save file path if not stated by call parameter
	if (strnlen(aSaveFilePath, ARRAYSIZE(aSaveFilePath)) == 0)
	{
		GetModuleFileName(NULL, aProgPathCropped, ARRAYSIZE(aProgPathCropped));
		pProgPathLastSlash = CCore::StringFindLastCharacter(aProgPathCropped, "\\");

		if (pProgPathLastSlash != NULL)
		{
			*pProgPathLastSlash = '\0';
		}
		else
		{
			printf("Unable to determine last slash ('\\') in program file path '%s'\n", aProgPathCropped);
			return ERROR;
		}

		snprintf(aSaveFilePath, ARRAYSIZE(aSaveFilePath), "%s\\" MAIN_SAVEFILE_NAME, aProgPathCropped);
	}

	CMainLogic mainLogic(paramStartmaximized, aSaveFilePath);
	return mainLogic.EntryPoint();
}