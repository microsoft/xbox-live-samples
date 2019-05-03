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
    SampleLog(LL_TRACE, "Initializing Identity Integration");
}

void IdentityMenu_Integration::updateIdentityButtons(int status)
{
    if ([IdentityMenuView shared]) {
        [[IdentityMenuView shared] updateSignInState:status];
    }
}

void IdentityMenu_Integration::updateIdentityImage(const char* imageUrl)
{
    if ([IdentityMenuView shared]) {
        if (imageUrl) {
            NSString* urlString = [NSString stringWithUTF8String:imageUrl];
            UIImage* userImage = [UIImage imageWithData:[NSData dataWithContentsOfURL:[NSURL URLWithString:urlString]]];
            [[IdentityMenuView shared] updateUserImageView:userImage];
        } else {
            [[IdentityMenuView shared] updateUserImageView:nil];
        }
    }
}

void IdentityMenu_Integration::updateIdentityTitle(const char* title)
{
    if ([IdentityMenuView shared])
    {
        if (title) {
            [[IdentityMenuView shared] updateUserIDLabel:[NSString stringWithUTF8String:title]];
        } else {
            [[IdentityMenuView shared] updateUserIDLabel:nil];
        }
    }
}

void IdentityMenu_Integration::updateIdentityGamerScore(const char* score)
{
    if ([IdentityMenuView shared])
    {
        if (score) {
            [[IdentityMenuView shared] updateUserGamerScore:[NSString stringWithUTF8String:score]];
        } else {
            [[IdentityMenuView shared] updateUserGamerScore:nil];
        }
    }
}
