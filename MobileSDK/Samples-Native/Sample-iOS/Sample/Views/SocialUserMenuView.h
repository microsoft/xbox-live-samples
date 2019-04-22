// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#import <UIKit/UIKit.h>

@interface SocialUserMenuView : UIView <UITableViewDelegate, UITableViewDataSource>

- (void)reset;
- (void)updateMenuHidden:(BOOL)hidden;
- (void)backToPreviousMenu;

@end
