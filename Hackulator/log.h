#pragma once

#include <stdarg.h>

// class defines
#define CLOG_LOG_MAXLEN 256

class CLog
{
public:
    enum E_CUSTOMFLAGS
    {
        CFL_NONEWLINE = (1 << 0),
    };

    CLog();
    ~CLog();

    void Log(const char* pMessage, ...);
    void LogErr(const char* pMessage, ...);
    void LogCustom(E_CUSTOMFLAGS Flags, const char* pMessage, ...);
    void LogCustom(E_CUSTOMFLAGS Flags, const char* pMessage, va_list Argptr);

private:
    enum E_LOGTYPES
    {
        INFO,
        ERR,
    };

    void LogBase(E_LOGTYPES Type, const char* pMessage, va_list Argptr);
};