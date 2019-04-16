// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <pch.h>
#include "XSAPI_Integration.h"
#include "Achievements_Integration.h"

#pragma region Achievements Integration
void CALLBACK Achievements_GetAchievementsForTitle_Callback(_In_ XAsyncBlock* asyncBlock)
{
    if (asyncBlock == nullptr) { return; }

    XblAchievementsResultHandle achievementsResultHandle = nullptr;
    HRESULT hr = XblAchievementsGetAchievementsForTitleIdResult(asyncBlock, &achievementsResultHandle);

    XblAchievement* achievements = nullptr;
    uint32_t achievementsCount = 0;
    bool hasNextPage = false;

    if (SUCCEEDED(hr))
    {
        SampleLog(LL_TRACE, "Successfully found achievements for this title!");

        hr = XblAchievementsResultGetAchievements(achievementsResultHandle, &achievements, &achievementsCount);

        if (SUCCEEDED(hr))
        {
            XblAchievementsResultHasNext(achievementsResultHandle, &hasNextPage);

            SampleLog(LL_TRACE, "Successfully got %i achievements!", achievementsCount);
        }
        else
        {
            SampleLog(LL_TRACE, "Failed at getting achievements!");
            SampleLog(LL_ERROR, "XblAchievementsResultGetAchievements failed!");
            SampleLog(LL_ERROR, "Error code: %s!", ConvertHRtoString(hr).c_str());
        }
    }
    else
    {
        SampleLog(LL_TRACE, "Failed at finding achievements for this title!");
        SampleLog(LL_ERROR, "XblAchievementsGetAchievementsForTitleIdResult failed!");
        SampleLog(LL_ERROR, "Error code: %s!", ConvertHRtoString(hr).c_str());
    }

    Achievements_Gameplay_GetAchievementsForTitle(hr, achievementsResultHandle, achievements, achievementsCount, hasNextPage);

    delete asyncBlock;
}

HRESULT Achievements_GetAchievementsForTitle(
    _In_ XTaskQueueHandle asyncQueue,
    _In_ XblContextHandle xblContext,
    _In_ uint32_t skipItems,
    _In_ uint32_t maxItems)
{
    if (xblContext == nullptr)
    {
        SampleLog(LL_ERROR, "XblContextHandle is NULL!");
        return E_INVALIDARG;
    }

    XAsyncBlock* asyncBlock = new XAsyncBlock();
    asyncBlock->queue = asyncQueue;
    asyncBlock->callback = Achievements_GetAchievementsForTitle_Callback;

    XalUserHandle user = nullptr;
    HRESULT hr = XblContextGetUser(xblContext, &user);

    if (FAILED(hr))
    {
        SampleLog(LL_ERROR, "XblContextGetUser failed!");
        SampleLog(LL_ERROR, "Error code: %s!", ConvertHRtoString(hr).c_str());
        return hr;
    }

    uint64_t xuid = 0;
    hr = XalUserGetId(user, &xuid);

    if (FAILED(hr))
    {
        SampleLog(LL_ERROR, "XalUserGetId failed!");
        SampleLog(LL_ERROR, "Error code: %s!", ConvertHRtoString(hr).c_str());
        return hr;
    }

    uint32_t titleId = 0;
    hr = XalGetTitleId(&titleId);

    if (FAILED(hr))
    {
        SampleLog(LL_ERROR, "XalGetTitleId failed!");
        SampleLog(LL_ERROR, "Error code: %s!", ConvertHRtoString(hr).c_str());
        return hr;
    }

    SampleLog(LL_TRACE, ""); // New Line
    SampleLog(LL_TRACE, "Requesting to get Achievements for title");

    return XblAchievementsGetAchievementsForTitleIdAsync(
        xblContext,
        xuid,
        titleId,
        XblAchievementType::All,
        false,
        XblAchievementOrderBy::DefaultOrder,
        skipItems,
        maxItems,
        asyncBlock);
}

void CALLBACK Achievements_GetNextResultsPage_Callback(_In_ XAsyncBlock* asyncBlock)
{
    if (asyncBlock == nullptr) return;

    XblAchievementsResultHandle achievementsResultHandle = nullptr;
    HRESULT hr = XblAchievementsResultGetNextResult(asyncBlock, &achievementsResultHandle);

    XblAchievement* achievements = nullptr;
    uint32_t achievementsCount = 0;
    bool hasNextPage = false;

    if (SUCCEEDED(hr))
    {
        SampleLog(LL_TRACE, "Successfully found next page of achievements!");

        hr = XblAchievementsResultGetAchievements(achievementsResultHandle, &achievements, &achievementsCount);

        if (SUCCEEDED(hr))
        {
            XblAchievementsResultHasNext(achievementsResultHandle, &hasNextPage);

            SampleLog(LL_TRACE, "Successfully got %i achievements!", achievementsCount);
        }
        else
        {
            SampleLog(LL_TRACE, "Failed at getting achievements!");
            SampleLog(LL_ERROR, "XblAchievementsResultGetAchievements failed!");
            SampleLog(LL_ERROR, "Error code: %s!", ConvertHRtoString(hr).c_str());
        }
    }
    else
    {
        SampleLog(LL_TRACE, "There is no next page of achievements!");
        SampleLog(LL_ERROR, "XblAchievementsResultGetNextResult failed!");
        SampleLog(LL_ERROR, "Error code: %s!", ConvertHRtoString(hr).c_str());
    }

    Achievements_Gameplay_GetNextResultsPage(hr, achievementsResultHandle, achievements, achievementsCount, hasNextPage);

    delete asyncBlock;
}

HRESULT Achievements_GetNextResultsPage(
    _In_ XTaskQueueHandle asyncQueue,
    _In_ XblContextHandle xblContext,
    _In_ XblAchievementsResultHandle achievementsResultHandle,
    _In_ uint32_t maxItems)
{
    if (xblContext == nullptr)
    {
        SampleLog(LL_ERROR, "XblContextHandle is NULL!");
        return E_INVALIDARG;
    }

    if (achievementsResultHandle == nullptr)
    {
        SampleLog(LL_ERROR, "XblAchievementsResultHandle is NULL!");
        return E_INVALIDARG;
    }

    XAsyncBlock* asyncBlock = new XAsyncBlock();
    asyncBlock->queue = asyncQueue;
    asyncBlock->callback = Achievements_GetNextResultsPage_Callback;

    SampleLog(LL_TRACE, ""); // New Line
    SampleLog(LL_TRACE, "Requesting to get next page of Achievements");

    return XblAchievementsResultGetNextAsync(
        xblContext,
        achievementsResultHandle,
        maxItems,
        asyncBlock);
}

void CALLBACK Achievements_GetAchievement_Callback(_In_ XAsyncBlock* asyncBlock)
{
    if (asyncBlock == nullptr) { return; }

    XblAchievementsResultHandle resultHandle = nullptr;
    HRESULT hr = XblAchievementsGetAchievementResult(asyncBlock, &resultHandle);

    XblAchievement* achievements = nullptr;
    uint32_t achievementsCount = 0;

    if (SUCCEEDED(hr))
    {
        SampleLog(LL_TRACE, "Successfully found achievement!");

        hr = XblAchievementsResultGetAchievements(resultHandle, &achievements, &achievementsCount);

        if (SUCCEEDED(hr))
        {
            SampleLog(LL_TRACE, "Successfully got %i achievement!", achievementsCount);
        }
        else
        {
            SampleLog(LL_TRACE, "Failed at getting achievement!");
            SampleLog(LL_ERROR, "XblAchievementsResultGetAchievements failed!");
            SampleLog(LL_ERROR, "Error code: %s!", ConvertHRtoString(hr).c_str());
        }
    }
    else
    {
        SampleLog(LL_TRACE, "Failed at finding achievement!");
        SampleLog(LL_ERROR, "XblAchievementsGetAchievementResult failed!");
        SampleLog(LL_ERROR, "Error code: %s!", ConvertHRtoString(hr).c_str());
    }

    Achievements_Gameplay_GetAchievement(hr, achievements, achievementsCount);

    delete asyncBlock;
}

HRESULT Achievements_GetAchievement(
    _In_ XTaskQueueHandle asyncQueue,
    _In_ XblContextHandle xblContext,
    _In_z_ const std::string& achievementId)
{
    if (xblContext == nullptr)
    {
        SampleLog(LL_ERROR, "XblContextHandle is NULL!");
        return E_INVALIDARG;
    }

    XAsyncBlock* asyncBlock = new XAsyncBlock();
    asyncBlock->queue = asyncQueue;
    asyncBlock->callback = Achievements_GetAchievement_Callback;

    XalUserHandle user = nullptr;
    HRESULT hr = XblContextGetUser(xblContext, &user);

    if (FAILED(hr))
    {
        SampleLog(LL_ERROR, "XblContextGetUser failed!");
        SampleLog(LL_ERROR, "Error code: %s!", ConvertHRtoString(hr).c_str());
        return hr;
    }

    uint64_t xuid = 0;
    hr = XalUserGetId(user, &xuid);

    if (FAILED(hr))
    {
        SampleLog(LL_ERROR, "XalUserGetId failed!");
        SampleLog(LL_ERROR, "Error code: %s!", ConvertHRtoString(hr).c_str());
        return hr;
    }

    XblGuid scid = {0};
    hr = XblGetScid(&scid);

    if (FAILED(hr))
    {
        SampleLog(LL_ERROR, "XblGetScid failed!");
        SampleLog(LL_ERROR, "Error code: %s!", ConvertHRtoString(hr).c_str());
        return hr;
    }

    SampleLog(LL_TRACE, ""); // New Line
    SampleLog(LL_TRACE, "Requesting to get Achievement %s", achievementId.c_str());

    return XblAchievementsGetAchievementAsync(
        xblContext,
        xuid,
        scid.value,
        achievementId.c_str(),
        asyncBlock);
}

void CALLBACK Achievements_UpdateAchievement_Callback(_In_ XAsyncBlock* asyncBlock)
{
    if (asyncBlock == nullptr) return;

    HRESULT hr = XAsyncGetStatus(asyncBlock, false);

    if (SUCCEEDED(hr))
    {
        SampleLog(LL_TRACE, "Successfully updated achievement!");
    }
    else if (hr == HTTP_E_STATUS_NOT_MODIFIED)
    {
        SampleLog(LL_TRACE, "Achievement not modified!");
        SampleLog(LL_TRACE, "Percent complete must be more than current!");
        SampleLog(LL_ERROR, "XAsyncGetStatus failed!");
        SampleLog(LL_ERROR, "Error code: %s!", ConvertHRtoString(hr).c_str());
    }
    else
    {
        SampleLog(LL_TRACE, "Failed updating achievement!");
        SampleLog(LL_ERROR, "XAsyncGetStatus failed!");
        SampleLog(LL_ERROR, "Error code: %s!", ConvertHRtoString(hr).c_str());
    }

    Achievements_Gameplay_UpdateAchievement(hr);

    delete asyncBlock;
}

HRESULT Achievements_UpdateAchievement(
    _In_ XTaskQueueHandle asyncQueue,
    _In_ XblContextHandle xblContext,
    _In_z_ const std::string& achievementId,
    _In_ uint32_t percentComplete)
{
    if (xblContext == nullptr)
    {
        SampleLog(LL_ERROR, "XblContextHandle is NULL!");
        return E_INVALIDARG;
    }

    XAsyncBlock* asyncBlock = new XAsyncBlock();
    asyncBlock->queue = asyncQueue;
    asyncBlock->callback = Achievements_UpdateAchievement_Callback;

    XalUserHandle user = nullptr;
    HRESULT hr = XblContextGetUser(xblContext, &user);

    if (FAILED(hr))
    {
        SampleLog(LL_ERROR, "XblContextGetUser failed!");
        SampleLog(LL_ERROR, "Error code: %s!", ConvertHRtoString(hr).c_str());
        return hr;
    }

    uint64_t xuid = 0;
    hr = XalUserGetId(user, &xuid);

    if (FAILED(hr))
    {
        SampleLog(LL_ERROR, "XalUserGetId failed!");
        SampleLog(LL_ERROR, "Error code: %s!", ConvertHRtoString(hr).c_str());
        return hr;
    }

    SampleLog(LL_TRACE, ""); // New Line
    SampleLog(LL_TRACE, "Requesting to update Achievement");
    SampleLog(LL_TRACE, "%s) %i/100", achievementId.c_str(), percentComplete);

    return XblAchievementsUpdateAchievementAsync(
        xblContext,
        xuid,
        achievementId.c_str(),
        percentComplete,
        asyncBlock);
}
#pragma endregion