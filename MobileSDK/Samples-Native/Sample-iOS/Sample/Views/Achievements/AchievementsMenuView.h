// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#import <UIKit/UIKit.h>

@class HubMenuView;

@interface AchievementsMenuView : UIView

@property (nonatomic, strong) HubMenuView* parentMenu;

- (void)setHasNextResultsPage:(BOOL)value;
- (BOOL)getHasNextResultsPage;
- (void)setAchievementsResultHandle:(XblAchievementsResultHandle)resultHandle;
- (XblAchievementsResultHandle)getAchievementsResultHandle;

@end
