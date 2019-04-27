#pragma once

#include <string>
#include <time.h>

inline Platform::String^ FormatHResult(int hResult)
{
    wchar_t hrBuf[11] = {};
    swprintf_s(hrBuf, 11, L"0x%08X", hResult);
    return ref new Platform::String(hrBuf);
}

inline Platform::String^ GetErrorStringForException(Platform::Exception^ ex)
{
    if (ex == nullptr)
    {
        return "Unknown error";
    }

    return FormatHResult(ex->HResult);
}

inline Platform::String^ FormatLocalTimeFromDateTime(const Windows::Foundation::DateTime& date)
{
    ULARGE_INTEGER largeTime;
    largeTime.QuadPart = static_cast<uint64>(date.UniversalTime); // UniversalTime is a 'long long' (a.k.a. int64)

    FILETIME* pFileTime = reinterpret_cast<FILETIME*>(&largeTime);

#ifdef _XBOX_ONE
    FILETIME localFileTime;
    FileTimeToLocalFileTime(pFileTime, &localFileTime); // convert from UTC to local time

    SYSTEMTIME localSystemTime;
    FileTimeToSystemTime(&localFileTime, &localSystemTime); // convert to SYSTEMTIME struct
#else
    SYSTEMTIME systemTime;
    FileTimeToSystemTime(pFileTime, &systemTime); // convert to SYSTEMTIME struct

    SYSTEMTIME localSystemTime;
    SystemTimeToTzSpecificLocalTime(nullptr, &systemTime, &localSystemTime); // convert to current time zone's local time
#endif

    wchar_t systemTimeStr[50];
    swprintf_s(systemTimeStr, 50, L"%u/%u/%u %02u:%02u:%02u",
        localSystemTime.wMonth, localSystemTime.wDay, localSystemTime.wYear,
        localSystemTime.wHour, localSystemTime.wMinute, localSystemTime.wSecond);

    return ref new Platform::String(systemTimeStr);
}

#ifdef _XBOX_ONE
inline Platform::String^ FormatUserName(Windows::Xbox::System::User^ user, bool includeXuid = true, bool returnEmptyStringForNullUser = false)
{
    if (user != nullptr && user->DisplayInfo != nullptr)
    {
        if (includeXuid)
        {
            return user->DisplayInfo->GameDisplayName + " (ID: " + user->Id + ")";
        }
        else
        {
            return user->DisplayInfo->GameDisplayName;
        }
    }
    else
    {
        if (returnEmptyStringForNullUser)
        {
            return "";
        }
        else
        {
            return "(unknown user)";
        }
    }
}
#else
inline Platform::String^ FormatUserName(std::shared_ptr<xbox::services::system::xbox_live_user> user, bool includeXuid = true, bool returnEmptyStringForNullUser = false)
{
    if (user != nullptr)
    {
        Platform::String^ gamertag = ref new Platform::String(user->gamertag().c_str());
        if (includeXuid)
        {
            return gamertag + " (ID: " + ref new Platform::String(user->xbox_user_id().c_str()) + ")";
        }
        else
        {
            return gamertag;
        }
    }
    else
    {
        if (user == nullptr)
        {
            Log::Write("user is null\n");
        }

        if (returnEmptyStringForNullUser)
        {
            return "";
        }
        else
        {
            return "(unknown)";
        }
    }
}
#endif

inline bool IsStringEqualCaseInsensitive(Platform::String^ val1, Platform::String^ val2)
{
    return (_wcsicmp(val1->Data(), val2->Data()) == 0);
}
