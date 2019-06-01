// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#import <UIKit/UIKit.h>

@class SocialMenuView;

@interface SocialUserMenuView : UIView <UITableViewDelegate, UITableViewDataSource>

@property (nonatomic, strong) SocialMenuView* parentMenu;

- (void)socialUserDisplayMenuExit;

- (void)updateUserImageView:(UIImage*)image;
- (void)updateUserIDLabel:(NSString*)title;
- (void)updateUserGamerScore:(NSString*)score;
- (void)updateUserStatus:(NSString*)status;
- (void)updateUserRelationship:(NSString*)relationship;

@end
