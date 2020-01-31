// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#define ID_NONE         0
#define ID_SIGNED_IN    1
#define ID_SIGNED_OUT   2

class IdentityMenu_Integration
{
public:
    void* identityMenuInstance;
    
    static IdentityMenu_Integration* getInstance();
    
    void init();
    
    void updateIdentityButtons(int status);
    void updateIdentityImage(const char* imageUrl);
    void updateIdentityTitle(const char* title);
    void updateIdentityGamerScore(const char* score);
    void updateIdentityContext(int status);
};
