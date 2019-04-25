// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import <UIKit/UIKit.h>

@interface SocialMenuView : UIView

+ (SocialMenuView*)shared;

- (void)reset;
- (void)updateMenuHidden:(BOOL)hidden;
- (void)backToPreviousMenu;

@end
