// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import "SocialMenuView.h"
#import <Game_Integration.h>
#import "SocialUserMenuView.h"
#import "SocialGroupMenuView.h"
#import "HubMenuView.h"

@interface SocialMenuView() {}

@property (nonatomic, weak) IBOutlet UIView* contentView;
@property (nonatomic, weak) IBOutlet UIButton* userButton;
@property (nonatomic, weak) IBOutlet UIButton* groupButton;
@property (nonatomic, weak) IBOutlet UIButton* backToMainButton;

@property (strong) SocialUserMenuView* socialUserMenuView;
@property (strong) SocialGroupMenuView* socialGroupMenuView;

@end

@implementation SocialMenuView

- (id)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        [self initialize];
    }
    return self;
}

- (id)initWithCoder:(NSCoder*)aDecoder {
    self = [super initWithCoder:aDecoder];
    if (self) {
        [self initialize];
    }
    return self;
}

- (void)initialize {
    [[NSBundle mainBundle] loadNibNamed:@"SocialMenuView" owner:self options:nil];
    [self addSubview:self.contentView];
    self.contentView.frame = self.bounds;

    self.parentMenu = nil;
    
    self.userButton.layer.borderWidth = 1.0f;
    self.userButton.layer.borderColor = [UIColor lightGrayColor].CGColor;
    self.userButton.layer.cornerRadius = 10.0f;

    self.groupButton.layer.borderWidth = 1.0f;
    self.groupButton.layer.borderColor = [UIColor lightGrayColor].CGColor;
    self.groupButton.layer.cornerRadius = 10.0f;

    self.backToMainButton.layer.borderWidth = 1.0f;
    self.backToMainButton.layer.borderColor = [UIColor lightGrayColor].CGColor;
    self.backToMainButton.layer.cornerRadius = 10.0f;
}

- (void)dealloc {
    SampleLog(LL_TRACE, "Social Menu dealloc.");
}

- (void)socialUserMenuExit {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (self.socialUserMenuView) {
            [self.socialUserMenuView removeFromSuperview];
            self.socialUserMenuView = nil;
        }
    });
}

- (void)socialGroupMenuExit {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (self.socialGroupMenuView) {
            [self.socialGroupMenuView removeFromSuperview];
            self.socialGroupMenuView = nil;
        }
    });
}

#pragma mark - IBActions

- (IBAction) userButtonTapped {
    SampleLog(LL_TRACE, "Social User tapped.");

    if (!self.socialUserMenuView) {
        dispatch_async(dispatch_get_main_queue(), ^{
            self.socialUserMenuView = [[SocialUserMenuView alloc] initWithFrame:self.bounds];
            self.socialUserMenuView.parentMenu = self;
            [self addSubview:self.socialUserMenuView];
        });
    }
}

- (IBAction) groupButtonTapped {
    SampleLog(LL_TRACE, "Social Group tapped.");

    if (!self.socialGroupMenuView) {
        dispatch_async(dispatch_get_main_queue(), ^{
            self.socialGroupMenuView = [[SocialGroupMenuView alloc] initWithFrame:self.bounds];
            self.socialGroupMenuView.parentMenu = self;
            [self addSubview:self.socialGroupMenuView];
        });
    }
}

- (IBAction) backToMainButtonTapped {
    SampleLog(LL_TRACE, "Social Back-To-Hub tapped.");
    
    if (self.parentMenu) {
        [self.parentMenu socialMenuExit];
    }
}

@end
