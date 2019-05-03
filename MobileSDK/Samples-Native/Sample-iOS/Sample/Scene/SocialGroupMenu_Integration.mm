// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "SocialGroupMenu_Integration.h"
#import <SocialGroupMenuView.h>

static SocialGroupMenu_Integration* s_socialGroupInstance = nullptr;

SocialGroupMenu_Integration* SocialGroupMenu_Integration::getInstance()
{
    if (!s_socialGroupInstance)
    {
        s_socialGroupInstance = new (std::nothrow) SocialGroupMenu_Integration();
        s_socialGroupInstance->init();
    }

    return s_socialGroupInstance;
}

void SocialGroupMenu_Integration::init()
{
    SampleLog(LL_TRACE, "Initializing Social Group Menu Integration");
}

void SocialGroupMenu_Integration::updateSocialGroupFriends(XblSocialManagerUserGroup* friends)
{
    [[SocialGroupMenuView shared] setSocialGroupFriends:friends];
}

void SocialGroupMenu_Integration::updateSocialGroupFavorites(XblSocialManagerUserGroup* favorites)
{
    [[SocialGroupMenuView shared] setSocialGroupFavorites:favorites];
}

void SocialGroupMenu_Integration::refreshSocialGroups()
{
    [[SocialGroupMenuView shared] refreshSocialGroups];
}
