// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <pch.h>
#include <cstdio>       //vsnprintf
#include <cstdarg>      //va_list, va_start, va_end
#include <string>       //std::string
#include <ConsoleView_JNI.h>
#include "SampleLog.h"

#define SCREENLOG_FORMAT_BUFFER_SIZE    8192

static char g_screenLogFormatBuffer[SCREENLOG_FORMAT_BUFFER_SIZE] = {0};

void SampleLog(int logLevel, const char* text, ...)
{
    va_list t_va;
    va_start(t_va, text);

    int size = vsnprintf(g_screenLogFormatBuffer, SCREENLOG_FORMAT_BUFFER_SIZE - 1, text, t_va);

    if (size >= 0)
    {
        ConsoleView_Log(logLevel, g_screenLogFormatBuffer);
    }

    va_end(t_va);
}