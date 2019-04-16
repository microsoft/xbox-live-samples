// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

HRESULT Identity_TrySignInUserSilently(_In_ XTaskQueueHandle asyncQueue);
HRESULT Identity_TrySignInUserWithUI(_In_ XTaskQueueHandle asyncQueue);
HRESULT Identity_TryResolveUserIssue(_In_ XTaskQueueHandle asyncQueue, _In_ XalUserHandle user);
HRESULT Identity_TrySignOutUser(_In_ XTaskQueueHandle asyncQueue, _In_ XalUserHandle user);
HRESULT Identity_GetDefaultGamerProfileAsync(_In_ XTaskQueueHandle asyncQueue, _In_ XblContextHandle contextHandle);
HRESULT Identity_GetGamerProfileAsync(_In_ XTaskQueueHandle asyncQueue, _In_ XblContextHandle contextHandle, _In_ uint64_t xboxUserId);

// TODO: Add in GetGamerTag, GetGamerPic, and GetGamerScore when all functions are fully supported by XAL, for now call GetGamerProfileAsync
HRESULT Identity_GetGamerTag(_In_ XblUserHandle user, _Out_ std::string* gamertag);

// Identity_Gameplay functions
void Identity_Gameplay_TrySignInUserSilently(_In_ XalUserHandle newUser, _In_ HRESULT result);
void Identity_Gameplay_TrySignInUserWithUI(_In_ XalUserHandle newUser, _In_ HRESULT result);
void Identity_Gameplay_TryResolveUserIssue(_In_ XalUserHandle user, _In_ HRESULT result);
void Identity_Gameplay_TrySignOutUser(_In_ HRESULT result);
void Identity_Gameplay_GetDefaultGamerProfile(_In_ XblUserProfile userProfile, _In_ HRESULT result);
void Identity_Gameplay_GetGamerProfile(_In_ XblUserProfile userProfile, _In_ HRESULT result);