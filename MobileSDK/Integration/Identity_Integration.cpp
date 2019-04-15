// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <pch.h>
#include "XSAPI_Integration.h"
#include "Identity_Integration.h"

#pragma region Identity Integration
void CALLBACK Identity_TrySignInUserSilently_Callback(_In_ XAsyncBlock* asyncBlock)
{
    // Note: XalTryAddDefaultUserSilentlyResult will add a Ref to the passed in XalUserHandle
    XalUserHandle newUser = nullptr;
    HRESULT hr = XalTryAddDefaultUserSilentlyResult(asyncBlock, &newUser);

    if (FAILED(hr))
    {
        SampleLog(LL_ERROR, "XalTryAddDefaultUserSilentlyResult Failed!");
        SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(hr).c_str());
    }

    Identity_Gameplay_TrySignInUserSilently(newUser, hr);

    // Close the Reference if one was created during XalTryAddDefaultUserSilentlyResult
    if (newUser) { XalUserCloseHandle(newUser); }

    delete asyncBlock;
}

HRESULT Identity_TrySignInUserSilently(_In_ XTaskQueueHandle asyncQueue)
{
    XAsyncBlock* asyncBlock = new XAsyncBlock();
    asyncBlock->queue = asyncQueue;
    asyncBlock->callback = Identity_TrySignInUserSilently_Callback;

    return XalTryAddDefaultUserSilentlyAsync(nullptr, asyncBlock);
}

void CALLBACK Identity_TrySignInUserWithUI_Callback(_In_ XAsyncBlock* asyncBlock)
{
    // Note: XalAddUserWithUiResult will add a Ref to the passed in XalUserHandle
    XalUserHandle newUser = nullptr;
    HRESULT hr = XalAddUserWithUiResult(asyncBlock, &newUser);

    if (FAILED(hr))
    {
        SampleLog(LL_ERROR, "XalAddUserWithUiResult Failed!");
        SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(hr).c_str());
    }

    Identity_Gameplay_TrySignInUserWithUI(newUser, hr);

    // Close the Reference if one was created during XalTryAddDefaultUserSilentlyResult
    if (newUser) { XalUserCloseHandle(newUser); }

    delete asyncBlock;
}

HRESULT Identity_TrySignInUserWithUI(_In_ XTaskQueueHandle asyncQueue)
{
    XAsyncBlock* asyncBlock = new XAsyncBlock();
    asyncBlock->queue = asyncQueue;
    asyncBlock->callback = Identity_TrySignInUserWithUI_Callback;

    return XalAddUserWithUiAsync(nullptr, asyncBlock);
}

void CALLBACK Identity_TryResolveUserIssue_Callback(_In_ XAsyncBlock* asyncBlock)
{
    HRESULT hr = XAsyncGetStatus(asyncBlock, false);

    if (FAILED(hr))
    {
        SampleLog(LL_ERROR, "XAsyncGetStatus for XalUserResolveIssueWithUiAsync Failed!");
        SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(hr).c_str());
    }

    XalUserHandle user = reinterpret_cast<XalUserHandle>(asyncBlock->context);

    Identity_Gameplay_TryResolveUserIssue(user, hr);

    delete asyncBlock;
}

HRESULT Identity_TryResolveUserIssue(_In_ XTaskQueueHandle asyncQueue, _In_ XalUserHandle user)
{
    // Make sure a valid user was passed in
    if (user == nullptr)
    {
        SampleLog(LL_ERROR, "The XalUserHandle passed in was NULL!");
        return E_INVALIDARG;
    }

    XAsyncBlock* asyncBlock = new XAsyncBlock();
    asyncBlock->queue = asyncQueue;
    asyncBlock->context = user;
    asyncBlock->callback = Identity_TryResolveUserIssue_Callback;

    return XalUserResolveIssueWithUiAsync(user, "https://www.xboxlive.com", asyncBlock);
}

void CALLBACK Identity_TrySignOutUser_Callback(_In_ XAsyncBlock* asyncBlock)
{
    HRESULT hr = XAsyncGetStatus(asyncBlock, false);

    if (FAILED(hr))
    {
        SampleLog(LL_ERROR, "XAsyncGetStatus for XalSignOutUserAsync Failed!");
        SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(hr).c_str());
    }

    Identity_Gameplay_TrySignOutUser(hr);

    delete asyncBlock;
}

HRESULT Identity_TrySignOutUser(_In_ XTaskQueueHandle asyncQueue, _In_ XalUserHandle user)
{
    XAsyncBlock* asyncBlock = new XAsyncBlock();
    asyncBlock->queue = asyncQueue;
    asyncBlock->callback = Identity_TrySignOutUser_Callback;

    return XalSignOutUserAsync(user, asyncBlock);
}

void CALLBACK Identity_GetDefaultGamerProfile_Callback(_In_ XAsyncBlock* asyncBlock)
{
    XblUserProfile profile;
    HRESULT hr = XblProfileGetUserProfileResult(asyncBlock, &profile);

    if (FAILED(hr))
    {
        SampleLog(LL_ERROR, "XblProfileGetUserProfileResult Failed!");
        SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(hr).c_str());
    }

    Identity_Gameplay_GetDefaultGamerProfile(profile, hr);

    delete asyncBlock;
}

HRESULT Identity_GetDefaultGamerProfileAsync(_In_ XTaskQueueHandle asyncQueue, _In_ XblContextHandle contextHandle)
{
    XAsyncBlock* asyncBlock = new XAsyncBlock();
    asyncBlock->queue = asyncQueue;
    asyncBlock->callback = Identity_GetDefaultGamerProfile_Callback;

    XalUserHandle user = nullptr;
    HRESULT hr = XblContextGetUser(contextHandle, &user);

    if (FAILED(hr))
    {
        SampleLog(LL_ERROR, "XblContextGetUser Failed!");
        SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(hr).c_str());

        return hr;
    }

    uint64_t xboxUserID = 0;
    hr = XalUserGetId(user, &xboxUserID);

    if (FAILED(hr))
    {
        SampleLog(LL_ERROR, "XalUserGetId Failed!");
        SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(hr).c_str());

        return hr;
    }

    return XblProfileGetUserProfileAsync(contextHandle, xboxUserID, asyncBlock);
}

void CALLBACK Identity_GetGamerProfile_Callback(_In_ XAsyncBlock* asyncBlock)
{
    XblUserProfile profile;
    HRESULT hr = XblProfileGetUserProfileResult(asyncBlock, &profile);

    if (FAILED(hr))
    {
        SampleLog(LL_ERROR, "XblProfileGetUserProfileResult Failed!");
        SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(hr).c_str());
    }

    Identity_Gameplay_GetGamerProfile(profile, hr);

    delete asyncBlock;
}

HRESULT Identity_GetGamerProfileAsync(_In_ XTaskQueueHandle asyncQueue, _In_ XblContextHandle contextHandle, _In_ uint64_t xboxUserId)
{
    XAsyncBlock* asyncBlock = new XAsyncBlock();
    asyncBlock->queue = asyncQueue;
    asyncBlock->callback = Identity_GetGamerProfile_Callback;

    return XblProfileGetUserProfileAsync(contextHandle, xboxUserId, asyncBlock);
}

// TODO: Add in GetGamerTag, GetGamerPic, and GetGamerScore when all functions are fully supported by XAL

#pragma endregion