// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import "SocialMenuView.h"
#import "SocialUserMenuView.h"
#import "SocialGroupMenuView.h"
#import <GameScene.h>

@interface SocialMenuView() {}

@property (nonatomic, weak) IBOutlet UIView *contentView;
@property (nonatomic, weak) IBOutlet UIButton *userButton;
@property (nonatomic, weak) IBOutlet UIButton *groupButton;
@property (nonatomic, weak) IBOutlet UIButton *backToMainButton;

@end

@implementation SocialMenuView

+ (SocialMenuView*)shared {
    static SocialMenuView *sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[SocialMenuView alloc] initWithFrame:CGRectZero];
    });
    return sharedInstance;
}

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
        [[SocialUserMenuView shared] backToPreviousMenu];
        [[SocialGroupMenuView shared] backToPreviousMenu];
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

    dispatch_async(dispatch_get_main_queue(), ^{
        [SocialUserMenuView shared].frame = self.bounds;
        [[SocialUserMenuView shared] reset];
        [self addSubview:[SocialUserMenuView shared]];
    });
}

- (IBAction) groupButtonTapped {
    SampleLog(LL_TRACE, "Social Group tapped.");

    dispatch_async(dispatch_get_main_queue(), ^{
        [SocialGroupMenuView shared].frame = self.bounds;
        [[SocialGroupMenuView shared] reset];
        [self addSubview:[SocialGroupMenuView shared]];
    });
}

- (IBAction) backToMainButtonTapped {
    SampleLog(LL_TRACE, "Social Back-To-Main tapped.");
    
    [self backToPreviousMenu];
}

@end
