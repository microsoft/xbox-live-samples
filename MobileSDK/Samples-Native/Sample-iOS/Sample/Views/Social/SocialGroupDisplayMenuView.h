// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#import <UIKit/UIKit.h>

@class SocialGroupMenuView;

@interface SocialGroupDisplayMenuView : UIView <UITableViewDelegate, UITableViewDataSource>

@property (nonatomic, strong) SocialGroupMenuView* parentMenu;

- (void)setSocialGroup:(XblSocialManagerUserGroup*)socialGroup;
- (void)refreshSocialGroup;

@end
