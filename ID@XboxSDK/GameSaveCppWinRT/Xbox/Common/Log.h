//--------------------------------------------------------------------------------------
// Log.h
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include <vector>

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
    void Write(const wchar_t *format, ...);

    // Writes to the debug log file AND sends formatted output to the in-game log display
    void WriteAndDisplay(const wchar_t *format, ...);

    // Thread-safe log
    extern std::vector<winrt::hstring> g_displayLog;
};
