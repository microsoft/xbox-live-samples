// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

class XblSocialManagerUserGroup;

class SocialGroupMenu_Integration
{
public:
    void* socialGroupMenuInstance;

    XblSocialManagerUserGroup* socialGroupFriends;
    XblSocialManagerUserGroup* socialGroupFavorite;
    
    static SocialGroupMenu_Integration* getInstance();

    void init();

    void updateSocialGroupFriends(XblSocialManagerUserGroup* friends);
    void updateSocialGroupFavorites(XblSocialManagerUserGroup* favorites);
    void refreshSocialGroups();
};
