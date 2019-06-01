// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import <Social_Integration.h>
#import <SocialGroupMenu_Integration.h>

#pragma region Social Gameplay

void Social_Gameplay_UpdateFriendsSocialGroup(
    _In_ XblSocialManagerUserGroup* friendsGroup)
{
    SampleLog(LL_TRACE, "Update Friends Social Group");

    SocialGroupMenu_Integration::getInstance()->updateSocialGroupFriends(friendsGroup);
}

void Social_Gameplay_UpdateFavoriteSocialGroup(
    _In_ XblSocialManagerUserGroup* favoriteGroup)
{
    SampleLog(LL_TRACE, "Update Favorite Social Group");

    SocialGroupMenu_Integration::getInstance()->updateSocialGroupFavorites(favoriteGroup);
}

void Social_Gameplay_RefreshSocialGroups()
{
    SampleLog(LL_TRACE, "Refresh Social Groups");

    SocialGroupMenu_Integration::getInstance()->refreshSocialGroups();
}

#pragma endregion
