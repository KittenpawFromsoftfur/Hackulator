#include <stdarg.h>
#include <stdio.h>
#include <string.h>

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
	LogBase("", "\n", pMessage, argptr);
	va_end(argptr);
}

void CLog::LogErr(const char* pMessage, ...)
{
	va_list argptr;
	va_start(argptr, pMessage);
	LogBase("Error: ", "\n", pMessage, argptr);
	va_end(argptr);
}

void CLog::LogCustom(const char *pPrefix, const char *pSuffix, const char* pMessage, ...)
{
	va_list argptr;
	va_start(argptr, pMessage);
	LogBase(pPrefix, pSuffix, pMessage, argptr);
	va_end(argptr);
}

void CLog::LogBase(const char* pPrefix, const char* pSuffix, const char* pMessage, va_list Argptr)
{
	char aBuf[CLOG_LOG_MAXLEN] = { 0 };
	char aPrefix[CLOG_LOG_MAXLEN] = { 0 };
	char aSuffix[CLOG_LOG_MAXLEN] = { 0 };

	vsprintf(aBuf, pMessage, Argptr);
	strncpy(aPrefix, pPrefix, ARRAYSIZE(aPrefix));
	strncpy(aSuffix, pSuffix, ARRAYSIZE(aSuffix));
	printf("%s%s%s", aPrefix, aBuf, aSuffix);
}