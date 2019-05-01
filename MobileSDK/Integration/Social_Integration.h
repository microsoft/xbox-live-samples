// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

void Social_AddUserToSocialManager(
    _In_ XalUserHandle user);

void Social_RemoveUserFromSocialManager(
    _In_ XalUserHandle user);

void Social_GetUsersForSocialGroup(
    _In_ XblSocialManagerUserGroup* group,
    _In_ uint32_t xboxSocialUsersCount,
    _Out_writes_(xboxSocialUsersCount) XblSocialManagerUser* xboxSocialUsers);

XblSocialManagerUserGroup* Social_CreateSocialGroupFromList(
    _In_ XalUserHandle user,
    _In_ uint64_t* xboxUserIdList,
    _In_ uint32_t xboxUserIdListCount);

XblSocialManagerUserGroup* Social_CreateSocialGroupFromFilters(
    _In_ XalUserHandle user,
    _In_ XblPresenceFilter presenceDetailLevel,
    _In_ XblRelationshipFilter filter);

void Social_UpdateSocialManager();

void Social_Gameplay_UpdateFriendsSocialGroup(
    _In_ XblSocialManagerUserGroup* friendsGroup);

void Social_Gameplay_UpdateFavoriteSocialGroup(
    _In_ XblSocialManagerUserGroup* favoriteGroup);

void Social_Gameplay_RefreshSocialGroups();
