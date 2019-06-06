// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#import <UIKit/UIKit.h>

@protocol SocialGroupDisplayDelegate

@optional
- (void)SocialUserTappedWithXboxId:(uint64_t)userId;

@end

@interface SocialGroupDisplayView : UITableView <UITableViewDelegate, UITableViewDataSource>

@property (weak) id <SocialGroupDisplayDelegate> socialGroupDisplayDelegate;

- (void)setSocialGroup:(XblSocialManagerUserGroup*)socialGroup;
- (void)refreshSocialGroup;

@end
