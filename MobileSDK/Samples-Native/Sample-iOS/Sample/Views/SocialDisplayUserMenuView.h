// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#import <UIKit/UIKit.h>

@interface SocialDisplayUserMenuView : UIView

+ (SocialDisplayUserMenuView*)shared;

- (void)reset;
- (void)backToPreviousMenu;

- (void)updateUserImageView:(UIImage*)image;
- (void)updateUserIDLabel:(NSString*)title;
- (void)updateUserGamerScore:(NSString*)score;
- (void)updateUserStatus:(NSString*)status ;
- (void)updateUserRelationship:(NSString *)relationship;

@end
