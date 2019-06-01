// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#import <UIKit/UIKit.h>

@class SocialMenuView;

@interface SocialGroupMenuView : UIView

@property (nonatomic, strong) SocialMenuView* parentMenu;

- (void)socialGroupDisplayMenuExit;

- (void)setSocialGroupFriends:(XblSocialManagerUserGroup*)friends;
- (void)setSocialGroupFavorites:(XblSocialManagerUserGroup*)favorites;
- (void)refreshSocialGroups;

@end
