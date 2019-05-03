// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#import <UIKit/UIKit.h>

@interface IdentityMenuView : UIView

+ (IdentityMenuView*)shared;

- (void)updateSignInState:(int)status;
- (void)updateUserImageView:(UIImage*)image;
- (void)updateUserIDLabel:(NSString*)title;
- (void)updateUserGamerScore:(NSString*)score;

@end
