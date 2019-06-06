// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

class IdentityDisplayMenu_Integration
{
public:
    void* identityDisplayMenuInstance;

    static IdentityDisplayMenu_Integration* getInstance();

    void init();

    void updateUserImage(const char* imageUrl);
    void updateUserTitle(const char* title);
    void updateUserGamerScore(const char* score);
    void updateUserStatus(const char* status);
    void updateUserRelationship(const char* relationship);
};
