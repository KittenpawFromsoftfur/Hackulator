#include <stdarg.h>
#include <stdio.h>

#include "log.h"
#include "core.h"

CLog::CLog()
{

}

CLog::~CLog()
{

}

void CLog::Log(const char* pMessage, ...)
{
	va_list argptr;
	va_start(argptr, pMessage);
	LogBase(INFO, pMessage, argptr);
	va_end(argptr);
}

void CLog::LogErr(const char* pMessage, ...)
{
	va_list argptr;
	va_start(argptr, pMessage);
	LogBase(ERR, pMessage, argptr);
	va_end(argptr);
}

void CLog::LogBase(E_LOGTYPES Type, const char* pMessage, va_list Argptr)
{
	char aBuf[CLOG_LOG_MAXLEN] = { 0 };
	char aPrefix[CLOG_LOG_MAXLEN] = { 0 };

	switch (Type)
	{
	case ERR:
		snprintf(aPrefix, ARRAYSIZE(aPrefix), "Error: ");
		break;

	case INFO:
		// no prefix
		break;
	}

	vsprintf(aBuf, pMessage, Argptr);
	printf("%s%s\n", aPrefix, aBuf);
}