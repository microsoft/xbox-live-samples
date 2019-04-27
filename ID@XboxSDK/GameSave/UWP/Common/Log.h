// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

#include <concurrent_vector.h>

#ifdef _DEBUG
#define ENABLE_LOGGING_TO_FILE
#endif

#define LOG_CREATE_NEW_ON_LAUNCH    1

// Logging for file and screen
namespace Log
{
    void Initialize();

    // WARNING: Clear is not a thread-safe operation
    void ClearDisplayLog();

    // Adds message to the display log without any timestamp formatting
    void PushToDisplayLog(const std::wstring& message);

    // Writes to the debug log file (if enabled)
    void Write(Platform::String^ format, ...);

    // Writes to the debug log file AND sends formatted output to the in-game log display
    void WriteAndDisplay(Platform::String^ format, ...);

    // Thread-safe log
    extern Concurrency::concurrent_vector<Platform::String^> g_displayLog;
};
