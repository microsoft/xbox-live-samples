// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#import <UIKit/UIKit.h>

@class HubMenuView;

@interface SocialMenuView : UIView

@property (nonatomic, strong) HubMenuView* parentMenu;

- (void)socialUserMenuExit;
- (void)socialGroupMenuExit;

@end
