// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#import <UIKit/UIKit.h>

@interface IdentityDisplayView : UIView

- (void)embedInView:(UIView*)view;

- (void)updateUserImageView:(UIImage*)image;
- (void)updateUserIDLabel:(NSString*)title;
- (void)updateUserGamerScore:(NSString*)score;
- (void)updateUserStatus:(NSString*)status;
- (void)updateUserRelationship:(NSString*)relationship;

@end
