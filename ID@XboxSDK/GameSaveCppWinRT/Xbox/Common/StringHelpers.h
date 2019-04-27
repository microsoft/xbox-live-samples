//--------------------------------------------------------------------------------------
// StringHelpers.h
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include <string>
#include <time.h>
#include <winrt\Windows.Foundation.h>
#include <winrt\Windows.Xbox.System.h>

inline winrt::hstring FormatHResult(int hResult)
{
    WCHAR hrBuf[11];
    swprintf_s(hrBuf, 11, L"0x%08X", hResult);
    return hrBuf;
}

inline winrt::hstring GetErrorStringForException(winrt::hresult_error const & ex)
{
    return FormatHResult(ex.code());
}

inline winrt::hstring FormatLocalTimeFromDateTime(const winrt::Windows::Foundation::DateTime& date)
{
    ULARGE_INTEGER largeTime;
    largeTime.QuadPart = *reinterpret_cast<const uint64_t*>(&date); // UniversalTime is a 'long long' (a.k.a. int64)

    FILETIME* pFileTime = reinterpret_cast<FILETIME*>(&largeTime);

    FILETIME localFileTime;
    FileTimeToLocalFileTime(pFileTime, &localFileTime); // convert from UTC to local time

    SYSTEMTIME localSystemTime;
    FileTimeToSystemTime(&localFileTime, &localSystemTime); // convert to SYSTEMTIME struct

    wchar_t systemTimeStr[50];
    swprintf_s(systemTimeStr, 50, L"%u/%u/%u %02u:%02u:%02u",
        localSystemTime.wMonth, localSystemTime.wDay, localSystemTime.wYear,
        localSystemTime.wHour, localSystemTime.wMinute, localSystemTime.wSecond);

    return systemTimeStr;
}
inline winrt::hstring FormatUserName(winrt::Windows::Xbox::System::User const & user, bool includeXuid = true, bool returnEmptyStringForNullUser = false)
{
    if (user != nullptr && user.DisplayInfo() != nullptr)
    {
        if (includeXuid)
        {
            wchar_t buffer[1024];
            swprintf_s(buffer, L"%s (ID: %i)", user.DisplayInfo().GameDisplayName().c_str(), user.Id());            
            return buffer;
        }
        else
        {
            return user.DisplayInfo().GameDisplayName().data();
        }
    }
    else
    {
        if (returnEmptyStringForNullUser)
        {
            return L"";
        }
        else
        {
            return L"(unknown user)";
        }
    }
}

bool IsStringEqualCaseInsensitive(winrt::hstring const & val1, winrt::hstring const & val2)
{
    return (_wcsicmp(val1.data(), val2.data()) == 0);
}
