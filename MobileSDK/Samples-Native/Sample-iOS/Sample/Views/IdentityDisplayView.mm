// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import "IdentityDisplayView.h"
//#import <Identity_Integration.h>
#import <GameScene.h>
//#import <IdentityMenu_Integration.h>

#define DEFAULT_ID_TITLE    @"GamerTag ?"

@interface IdentityDisplayView() {}

@property (nonatomic, weak) IBOutlet UIView *contentView;
@property (nonatomic, weak) IBOutlet UIImageView *userImageView;
@property (nonatomic, weak) IBOutlet UILabel *userIDLabel;
@property (nonatomic, weak) IBOutlet UILabel *userGamerScoreLabel;
@property (nonatomic, weak) IBOutlet UILabel *userRelationshipLabel;
@property (nonatomic, weak) IBOutlet UILabel *userStatusLabel;

@end

@implementation IdentityDisplayView

- (id)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        [self initialize];
    }
    return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder {
    self = [super initWithCoder:aDecoder];
    if (self) {
        [self initialize];
    }
    return self;
}

- (void)initialize {
    [[NSBundle mainBundle] loadNibNamed:@"IdentityDisplayView" owner:self options:nil];
    [self addSubview:self.contentView];
    self.contentView.frame = self.bounds;

    self.userImageView.layer.cornerRadius = self.userImageView.bounds.size.width / 2.0;
    
    [self updateUserImageView:nil];
    [self updateUserIDLabel:nil];
    [self updateUserGamerScore:@"0"];
    [self updateUserRelationship:@"?"];
}

- (void)dealloc {
    SampleLog(LL_TRACE, "Identity Display dealloc!!!!");
}

- (void)embedInView:(UIView*)parentView {
    if (!parentView) return;
    
    self.translatesAutoresizingMaskIntoConstraints = NO;
    [parentView addSubview:self];

    NSLayoutConstraint *leading = [NSLayoutConstraint constraintWithItem:self
                                                               attribute:NSLayoutAttributeLeading
                                                               relatedBy:NSLayoutRelationEqual
                                                                  toItem:parentView
                                                               attribute:NSLayoutAttributeLeading
                                                              multiplier:1.0f
                                                                constant:0.0f];
    NSLayoutConstraint *trailing = [NSLayoutConstraint constraintWithItem:self
                                                                attribute:NSLayoutAttributeTrailing
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem:parentView
                                                                attribute:NSLayoutAttributeTrailing
                                                               multiplier:1.0f
                                                                 constant:0.0f];
    NSLayoutConstraint *top = [NSLayoutConstraint constraintWithItem:self
                                                           attribute:NSLayoutAttributeTop
                                                           relatedBy:NSLayoutRelationEqual
                                                              toItem:parentView
                                                           attribute:NSLayoutAttributeTop
                                                          multiplier:1.0f
                                                            constant:0.0f];
    NSLayoutConstraint *bottom = [NSLayoutConstraint constraintWithItem:self
                                                              attribute:NSLayoutAttributeBottom
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem:parentView
                                                              attribute:NSLayoutAttributeBottom
                                                             multiplier:1.0f
                                                               constant:0.0f];
    [parentView addConstraint:leading];
    [parentView addConstraint:trailing];
    [parentView addConstraint:top];
    [parentView addConstraint:bottom];
}

- (void)updateUserImageView:(UIImage*)image {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (image == nil) {
            self.userImageView.image = [UIImage imageNamed:@"xboxIcon"];
            return;
        }
        
        self.userImageView.image = image;
    });
}

- (void)updateUserIDLabel:(NSString*)title {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (title == nil) {
            self.userIDLabel.text = DEFAULT_ID_TITLE;
            return;
        }
    
        self.userIDLabel.text = title;
    });
}

- (void)updateUserGamerScore:(NSString*)score {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (score == nil) {
            self.userGamerScoreLabel.text = @"Gamer Score: ?";
            return;
        }
        
        self.userGamerScoreLabel.text = [NSString stringWithFormat:@"Gamer Score: %@", score];
    });
}

- (void)updateUserStatus:(NSString*)status {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (status == nil) {
            self.userStatusLabel.text = @"Status: ?";
            return;
        }
        
        self.userStatusLabel.text = [NSString stringWithFormat:@"Status: %@", status];
    });
}

- (void)updateUserRelationship:(NSString*)relationship {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (relationship == nil) {
            self.userRelationshipLabel.text = @"Relationship: ?";
            return;
        }
        
        self.userRelationshipLabel.text = [NSString stringWithFormat:@"Relationship: %@", relationship];
    });
}

@end
