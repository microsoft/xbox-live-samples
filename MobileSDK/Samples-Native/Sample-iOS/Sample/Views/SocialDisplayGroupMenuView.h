// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#import <UIKit/UIKit.h>

@interface SocialDisplayGroupMenuView : UIView <UITableViewDelegate, UITableViewDataSource>

+ (SocialDisplayGroupMenuView*)shared;

- (void)reset;
- (void)backToPreviousMenu;
- (void)setSocialGroup:(XblSocialManagerUserGroup*)socialGroup;
- (void)refreshSocialGroup;

@end
