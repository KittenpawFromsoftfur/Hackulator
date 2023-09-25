#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

#include "main.h"
#include "mainlogic.h"

/* TODO
	Implement infix-to-postfix function (shunting yard algorithm)
	Implement postfix-notation-resolver (reverse polish notation)
	Result with n leading zeroes (settings)
	Result with Datatype 1-8 Byte (settigns)
	Result with signed 2's complement (settings)
	help
		(or readme) list operator priority and associativity
		should explain every single aspect of the hackulator (help file cause too complicated? generatehelp?)
	Readme with help and thanks
		https://en.wikipedia.org/wiki/Reverse_Polish_notation
		https://en.wikipedia.org/wiki/Shunting_yard_algorithm
		https://mathcenter.oxford.emory.edu/site/cs171/shuntingYardAlgorithm/
		http://csis.pace.edu/~wolf/CS122/infix-postfix.htm
		https://en.wikipedia.org/wiki/Operator_associativity
		https://en.wikipedia.org/wiki/Operators_in_C_and_C%2B%2B#Operator_precedence
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