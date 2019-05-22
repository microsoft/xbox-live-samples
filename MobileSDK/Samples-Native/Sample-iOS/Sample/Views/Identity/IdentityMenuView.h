//
//  IdentityMenuView.h
//  Sample
//
//  Created by Xlive XdevX on 4/2/19.
//  Copyright © 2019 Microsoft Xbox Live. All rights reserved.
//

#pragma once

#import <UIKit/UIKit.h>

@interface IdentityMenuView : UIView

- (void)updateIdentityButtons:(int)status;
- (void)updateUserImageView:(UIImage*)image;
- (void)updateUserIDLabel:(NSString*)title;
- (void)updateUserGamerScore:(NSString*)score;

@end
