// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import "IdentityDisplayMenu_Integration.h"
#import <IdentityDisplayMenuView.h>

static IdentityDisplayMenu_Integration* s_socialUserInstance = nullptr;

IdentityDisplayMenu_Integration* IdentityDisplayMenu_Integration::getInstance()
{
    if (!s_socialUserInstance)
    {
        s_socialUserInstance = new (std::nothrow) IdentityDisplayMenu_Integration();
        s_socialUserInstance->init();
    }

    return s_socialUserInstance;
}

void IdentityDisplayMenu_Integration::init()
{
    SampleLog(LL_TRACE, "Initializing Identity Display Menu Integration");
}

void IdentityDisplayMenu_Integration::updateUserImage(const char* imageUrl)
{
    if (this->identityDisplayMenuInstance)
    {
        IdentityDisplayMenuView* identityDisplayMenuView = (__bridge IdentityDisplayMenuView*)this->identityDisplayMenuInstance;
        if (imageUrl) {
            NSString* urlString = [NSString stringWithUTF8String:imageUrl];
            UIImage* userImage = [UIImage imageWithData:[NSData dataWithContentsOfURL:[NSURL URLWithString:urlString]]];
            [identityDisplayMenuView updateUserImageView:userImage];
        } else {
            [identityDisplayMenuView updateUserImageView:nil];
        }
    }
}

void IdentityDisplayMenu_Integration::updateUserTitle(const char* title)
{
    if (this->identityDisplayMenuInstance)
    {
        IdentityDisplayMenuView* identityDisplayMenuView = (__bridge IdentityDisplayMenuView*)this->identityDisplayMenuInstance;
        if (title) {
            [identityDisplayMenuView updateUserIDLabel:[NSString stringWithUTF8String:title]];
        } else {
            [identityDisplayMenuView updateUserIDLabel:nil];
        }
    }
}

void IdentityDisplayMenu_Integration::updateUserGamerScore(const char* score)
{
    if (this->identityDisplayMenuInstance)
    {
        IdentityDisplayMenuView* identityDisplayMenuView = (__bridge IdentityDisplayMenuView*)this->identityDisplayMenuInstance;
        if (score) {
            [identityDisplayMenuView updateUserGamerScore:[NSString stringWithUTF8String:score]];
        } else {
            [identityDisplayMenuView updateUserGamerScore:nil];
        }
    }
}

void IdentityDisplayMenu_Integration::updateUserStatus(const char* status)
{
    //if (this->identityDisplayMenuInstance)
    //{
    //    IdentityDisplayMenuView* identityDisplayMenuView = (__bridge IdentityDisplayMenuView*)this->identityDisplayMenuInstance;
    //}
}

void IdentityDisplayMenu_Integration::updateUserRelationship(const char* relationship)
{
    //if (this->identityDisplayMenuInstance)
    //{
    //    IdentityDisplayMenuView* identityDisplayMenuView = (__bridge IdentityDisplayMenuView*)this->identityDisplayMenuInstance;
    //}
}

