// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import "ScreenLog.h"
#import "TextViewLog.h"

#define SCREENLOG_PRINT_BUFFER_SIZE     8192                        // The maximum total length of one log message.
char g_logMessageBuffer[SCREENLOG_PRINT_BUFFER_SIZE];

void SampleLog(int logLevel, const char *format, va_list args)
{
    if (format == nullptr) { return; }
    
    vsnprintf(g_logMessageBuffer, SCREENLOG_PRINT_BUFFER_SIZE - 1, format, args);
    
    NSString *logString = [NSString stringWithUTF8String:g_logMessageBuffer];
    [[TextViewLog shared] log:logLevel format:logString];
}

void SampleLog(int logLevel, const char* format, ...)
{
    va_list args;
    va_start (args, format);
    SampleLog(logLevel, format, args);
    va_end (args);
}

