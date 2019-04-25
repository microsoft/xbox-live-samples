// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import <UIKit/UIKit.h>

@interface AchievementsMenuView : UIView

+ (AchievementsMenuView*)shared;

- (void)reset;
- (void)setHasNextResultsPage:(BOOL)value;
- (BOOL)getHasNextResultsPage;
- (void)setAchievementsResultHandle:(XblAchievementsResultHandle)resultHandle;
- (XblAchievementsResultHandle)getAchievementsResultHandle;
- (void)backToPreviousMenu;

@end
