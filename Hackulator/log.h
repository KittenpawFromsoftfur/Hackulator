#pragma once

#include <stdarg.h>

// class defines
#define CLOG_LOG_MAXLEN 256

class CLog
{
public:
    enum E_CUSTOMFLAGS
    {
        CFL_NONEWL,
    };

    CLog();
    ~CLog();

    void Log(const char* pMessage, ...);
    void LogErr(const char* pMessage, ...);
    void LogCustom(const char* pPrefix, const char* pSuffix, const char* pMessage, ...);

private:
    void LogBase(const char *pPrefix, const char *pSuffix, const char* pMessage, va_list Argptr);
};