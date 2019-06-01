// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import "SocialUserMenu_Integration.h"
#import <SocialUserMenuView.h>

static SocialUserMenu_Integration* s_socialUserInstance = nullptr;

SocialUserMenu_Integration* SocialUserMenu_Integration::getInstance()
{
    if (!s_socialUserInstance)
    {
        s_socialUserInstance = new (std::nothrow) SocialUserMenu_Integration();
        s_socialUserInstance->init();
    }

    return s_socialUserInstance;
}

void SocialUserMenu_Integration::init()
{
    SampleLog(LL_TRACE, "Initializing Social User Menu Integration");
}

void SocialUserMenu_Integration::updateUserImage(const char* imageUrl)
{
    if (this->socialUserMenuInstance)
    {
        SocialUserMenuView* socialUserMenuView = (__bridge SocialUserMenuView*)this->socialUserMenuInstance;
        if (imageUrl) {
            NSString* urlString = [NSString stringWithUTF8String:imageUrl];
            UIImage* userImage = [UIImage imageWithData:[NSData dataWithContentsOfURL:[NSURL URLWithString:urlString]]];
            [socialUserMenuView updateUserImageView:userImage];
        } else {
            [socialUserMenuView updateUserImageView:nil];
        }
    }
}

void SocialUserMenu_Integration::updateUserTitle(const char* title)
{
    if (this->socialUserMenuInstance)
    {
        SocialUserMenuView* socialUserMenuView = (__bridge SocialUserMenuView*)this->socialUserMenuInstance;
        if (title) {
            [socialUserMenuView updateUserIDLabel:[NSString stringWithUTF8String:title]];
        } else {
            [socialUserMenuView updateUserIDLabel:nil];
        }
    }
}

void SocialUserMenu_Integration::updateUserGamerScore(const char* score)
{
    if (this->socialUserMenuInstance)
    {
        SocialUserMenuView* socialUserMenuView = (__bridge SocialUserMenuView*)this->socialUserMenuInstance;
        if (score) {
            [socialUserMenuView updateUserGamerScore:[NSString stringWithUTF8String:score]];
        } else {
            [socialUserMenuView updateUserGamerScore:nil];
        }
    }
}

void SocialUserMenu_Integration::updateUserStatus(const char* status)
{
    //if (this->socialUserMenuInstance)
    //{
    //    SocialUserMenuView* socialUserMenuView = (__bridge SocialUserMenuView*)this->socialUserMenuInstance;
    //}
}

void SocialUserMenu_Integration::updateUserRelationship(const char* relationship)
{
    //if (this->socialUserMenuInstance)
    //{
    //    SocialUserMenuView* socialUserMenuView = (__bridge SocialUserMenuView*)this->socialUserMenuInstance;
    //}
}

