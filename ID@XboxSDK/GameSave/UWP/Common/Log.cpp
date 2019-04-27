// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "pch.h"
#include "Log.h"
#include <mutex>

using namespace Concurrency;

/* extern */
concurrent_vector<Platform::String^> Log::g_displayLog;

namespace
{
    const size_t    c_logBufferSize = 8192;
    const wchar_t*  c_logEntryPrefix = L"WORDGAME: ";
    const wchar_t*  c_logFilename = L"WordGameDebugLog.txt";
    std::wstring    g_logFilePath;
    std::mutex      g_logMutex;

    // Writes message to debug output, prefixing message with c_logEntryPrefix
    // If ENABLE_LOGGING_TO_FILE is defined, writes message to c_logFilename, prefixing with timestamp
    // Returns new string containing message prefixed with timestamp
    std::wstring WriteToDebugOutputAndLogFile(const wchar_t* message)
    {
        std::lock_guard<std::mutex> lock(g_logMutex);

        OutputDebugString(c_logEntryPrefix);
        OutputDebugString(message);

        // note the log time
        SYSTEMTIME localTime;
        GetLocalTime(&localTime);

        wchar_t timeStr[100];
        swprintf_s(timeStr, 100, L"%02u:%02u:%02u.%03u ",
            localTime.wHour,
            localTime.wMinute,
            localTime.wSecond,
            localTime.wMilliseconds); // format: "hh:mm:ss.ms"

#ifdef ENABLE_LOGGING_TO_FILE
        FILE *file = nullptr;

        errno_t err = _wfopen_s(
            &file,
            g_logFilePath.c_str(),
            L"at+, ccs=UTF-8"
        );

        if (err != 0)
        {
            std::wstring err_msg(L"ERROR: Unable to open log file (code ");
            err_msg += std::to_wstring(err);
            err_msg += L")\n";
            OutputDebugString(err_msg.c_str());
        }
        else
        {
            auto fileStr = std::to_wstring(GetCurrentThreadId());
            fileStr += L" \t";
            fileStr += timeStr;
            fileStr += message;

            fwrite(
                (void*)fileStr.c_str(),
                sizeof(wchar_t),
                fileStr.length(),
                file
            );

            fflush(file);
            fclose(file);
        }
#endif

        // assemble return line
        std::wstring returnLine(timeStr);
        returnLine += message;

        return returnLine;
    }

} // end unnamed namespace


void Log::Initialize()
{
#ifdef ENABLE_LOGGING_TO_FILE

    // set log path
#if defined(_XBOX_ONE) && defined(_TITLE)
    Platform::String^ logFilePath = "D:";
#else
    Platform::String^ logFilePath;
    if (Windows::System::Profile::AnalyticsInfo::VersionInfo->DeviceFamily == "Windows.Xbox")
        logFilePath = "D:";
    else
        logFilePath = Windows::Storage::ApplicationData::Current->TemporaryFolder->Path;
#endif
    logFilePath += "\\" + ref new Platform::String(c_logFilename);
    g_logFilePath = std::wstring(logFilePath->Data());
    OutputDebugString(L"Log file path: ");
    OutputDebugString(g_logFilePath.c_str());
    OutputDebugString(L"\n");

#if LOG_CREATE_NEW_ON_LAUNCH == 1
    // delete previous log file, ignoring failures
    _wremove(g_logFilePath.c_str());
#endif

#endif
}

void Log::ClearDisplayLog()
{
    g_displayLog.clear();
}

void Log::PushToDisplayLog(const std::wstring& message)
{
    g_displayLog.push_back(ref new Platform::String(message.c_str()));
}

void Log::Write(Platform::String^ format, ...)
{
    static wchar_t msgbuffer[c_logBufferSize];

    va_list args;
    va_start(args, format);
    vswprintf(msgbuffer, c_logBufferSize, format->Data(), args);
    va_end(args);

    WriteToDebugOutputAndLogFile(msgbuffer);
}

void Log::WriteAndDisplay(Platform::String^ format, ...)
{
    static wchar_t msgbuffer[c_logBufferSize];

    va_list args;
    va_start(args, format);
    vswprintf(msgbuffer, c_logBufferSize, format->Data(), args);
    va_end(args);

    PushToDisplayLog(WriteToDebugOutputAndLogFile(msgbuffer));
}
