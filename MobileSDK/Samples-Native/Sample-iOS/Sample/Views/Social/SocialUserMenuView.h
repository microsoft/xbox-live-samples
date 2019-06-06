// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#import <UIKit/UIKit.h>
#import "SocialGroupDisplayView.h"

@class SocialMenuView;

@interface SocialUserMenuView : UIView <SocialGroupDisplayDelegate>

@property (nonatomic, strong) SocialMenuView* parentMenu;

- (void)identityDisplayMenuExit;

@end
