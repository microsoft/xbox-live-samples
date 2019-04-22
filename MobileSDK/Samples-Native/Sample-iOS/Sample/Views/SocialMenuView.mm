// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import "SocialMenuView.h"
#import "SocialUserMenuView.h"
#import "SocialGroupMenuView.h"
//#import <Achievements_Integration.h>
#import <GameScene.h>
//#import <AchievementsMenu_Integration.h>

@interface SocialMenuView() {}

@property (nonatomic, weak) IBOutlet UIView *contentView;
@property (nonatomic, weak) IBOutlet UIButton *userButton;
@property (nonatomic, weak) IBOutlet UIButton *groupButton;
@property (nonatomic, weak) IBOutlet UIButton *backToMainButton;

@property (strong) SocialUserMenuView *socialUserMenuView;
@property (strong) SocialGroupMenuView *socialGroupMenuView;

@end

@implementation SocialMenuView

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
    [[NSBundle mainBundle] loadNibNamed:@"SocialMenuView" owner:self options:nil];
    [self addSubview:self.contentView];
    self.contentView.frame = self.bounds;
    
    self.userButton.layer.borderWidth = 1.0f;
    self.userButton.layer.borderColor = [UIColor lightGrayColor].CGColor;
    self.userButton.layer.cornerRadius = 10.0f;

    self.groupButton.layer.borderWidth = 1.0f;
    self.groupButton.layer.borderColor = [UIColor lightGrayColor].CGColor;
    self.groupButton.layer.cornerRadius = 10.0f;

    self.backToMainButton.layer.borderWidth = 1.0f;
    self.backToMainButton.layer.borderColor = [UIColor lightGrayColor].CGColor;
    self.backToMainButton.layer.cornerRadius = 10.0f;

    [self reset];
    
    //AchievementsMenu_Integration::getInstance()->achievementsMenuInstance = (void *)CFBridgingRetain(self);
}

- (void)dealloc {
    SampleLog(LL_TRACE, "Social Menu dealloc!!!!");
}

- (void)reset {
}

- (void)updateMenuHidden:(BOOL)hidden {
    if (hidden) {
        if (self.socialUserMenuView) {
            [self.socialUserMenuView backToPreviousMenu];
        }

        if (self.socialGroupMenuView) {
            [self.socialGroupMenuView backToPreviousMenu];
        }
    }

    dispatch_async(dispatch_get_main_queue(), ^{
        self.hidden = hidden;
    });
}

- (void)backToPreviousMenu {
    dispatch_async(dispatch_get_main_queue(), ^{
        [self removeFromSuperview];
    });
}

#pragma mark - IBActions

- (IBAction) userButtonTapped {
    SampleLog(LL_TRACE, "Social User tapped.");

    if (!self.socialUserMenuView) {
        self.socialUserMenuView = [[SocialUserMenuView alloc] initWithFrame:self.bounds];
    }

    dispatch_async(dispatch_get_main_queue(), ^{
        [self.socialUserMenuView reset];
        [self addSubview:self.socialUserMenuView];
    });
}

- (IBAction) groupButtonTapped {
    SampleLog(LL_TRACE, "Social Group tapped.");

    if (!self.socialGroupMenuView) {
        self.socialGroupMenuView = [[SocialGroupMenuView alloc] initWithFrame:self.bounds];
    }

    dispatch_async(dispatch_get_main_queue(), ^{
        [self.socialGroupMenuView reset];
        [self addSubview:self.socialGroupMenuView];
    });
}

- (IBAction) backToMainButtonTapped {
    SampleLog(LL_TRACE, "Social Back-To-Main tapped.");
    
    [self backToPreviousMenu];
}

@end
