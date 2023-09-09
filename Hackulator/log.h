#pragma once

#include <stdarg.h>

// class defines
#define CLOG_LOG_MAXLEN 256

class CLog
{
public:
    CLog();
    ~CLog();

    void Log(const char* pMessage, ...);
    void LogErr(const char* pMessage, ...);

private:
    enum E_LOGTYPES
    {
        ERR,
        INFO,
    };

    void LogBase(E_LOGTYPES Type, const char* pMessage, va_list Argptr);
};