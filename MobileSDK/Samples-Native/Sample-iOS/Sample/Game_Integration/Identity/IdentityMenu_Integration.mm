// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import "IdentityMenu_Integration.h"
#import <IdentityMenuView.h>

static IdentityMenu_Integration* s_identityInstance = nullptr;

IdentityMenu_Integration* IdentityMenu_Integration::getInstance()
{
    if (!s_identityInstance)
    {
        s_identityInstance = new (std::nothrow) IdentityMenu_Integration();
        s_identityInstance->init();
    }
    
    return s_identityInstance;
}

void IdentityMenu_Integration::init()
{
    SampleLog(LL_TRACE, "Initializing Menu Integration");
}

void IdentityMenu_Integration::updateIdentityButtons(int status)
{
    if (this->identityMenuInstance)
    {
        IdentityMenuView* idView = (__bridge IdentityMenuView*)this->identityMenuInstance;
        [idView updateIdentityButtons:status];
    }
}

void IdentityMenu_Integration::updateIdentityImage(const char* imageUrl)
{
    if (this->identityMenuInstance)
    {
        IdentityMenuView* idView = (__bridge IdentityMenuView*)this->identityMenuInstance;
        if (imageUrl) {
            NSString* urlString = [NSString stringWithUTF8String:imageUrl];
            UIImage* userImage = [UIImage imageWithData:[NSData dataWithContentsOfURL:[NSURL URLWithString:urlString]]];
            [idView updateUserImageView:userImage];
        } else {
            [idView updateUserImageView:nil];
        }
    }
}

void IdentityMenu_Integration::updateIdentityTitle(const char* title)
{
    if (this->identityMenuInstance)
    {
        IdentityMenuView* idView = (__bridge IdentityMenuView*)this->identityMenuInstance;
        if (title) {
            [idView updateUserIDLabel:[NSString stringWithUTF8String:title]];
        } else {
            [idView updateUserIDLabel:nil];
        }
    }
}

void IdentityMenu_Integration::updateIdentityGamerScore(const char* score)
{
    if (this->identityMenuInstance)
    {
        IdentityMenuView* idView = (__bridge IdentityMenuView*)this->identityMenuInstance;
        if (score) {
            [idView updateUserGamerScore:[NSString stringWithUTF8String:score]];
        } else {
            [idView updateUserGamerScore:nil];
        }
    }
}
