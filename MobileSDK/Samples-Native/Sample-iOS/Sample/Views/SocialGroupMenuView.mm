// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import "SocialGroupMenuView.h"
//#import <Achievements_Integration.h>
#import <GameScene.h>
#import "SocialDisplayGroupMenuView.h"
//#import <AchievementsMenu_Integration.h>

@interface SocialGroupMenuView() {}

@property (nonatomic, weak) IBOutlet UIView *contentView;
@property (nonatomic, weak) IBOutlet UIButton *friendsGroupButton;
@property (nonatomic, weak) IBOutlet UIButton *favoritesGroupButton;
@property (nonatomic, weak) IBOutlet UIButton *backToSocialButton;

@property (strong) SocialDisplayGroupMenuView *socialDisplayGroupView;

@end

@implementation SocialGroupMenuView

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
    [[NSBundle mainBundle] loadNibNamed:@"SocialGroupMenuView" owner:self options:nil];
    [self addSubview:self.contentView];
    self.contentView.frame = self.bounds;
    
    self.friendsGroupButton.layer.borderWidth = 1.0f;
    self.friendsGroupButton.layer.borderColor = [UIColor lightGrayColor].CGColor;
    self.friendsGroupButton.layer.cornerRadius = 10.0f;

    self.favoritesGroupButton.layer.borderWidth = 1.0f;
    self.favoritesGroupButton.layer.borderColor = [UIColor lightGrayColor].CGColor;
    self.favoritesGroupButton.layer.cornerRadius = 10.0f;

    self.backToSocialButton.layer.borderWidth = 1.0f;
    self.backToSocialButton.layer.borderColor = [UIColor lightGrayColor].CGColor;
    self.backToSocialButton.layer.cornerRadius = 10.0f;

    [self reset];
    
    //AchievementsMenu_Integration::getInstance()->achievementsMenuInstance = (void *)CFBridgingRetain(self);
}

- (void)dealloc {
    SampleLog(LL_TRACE, "Social-Group Menu dealloc!!!!");
}

- (void)reset {
}

- (void)updateMenuHidden:(BOOL)hidden {
    if (hidden) {
        if (self.socialDisplayGroupView) {
            [self.socialDisplayGroupView backToPreviousMenu];
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

- (IBAction) friendsGroupButtonTapped {
    SampleLog(LL_TRACE, "Social-Group Friends Group tapped.");

    if (!self.socialDisplayGroupView) {
        self.socialDisplayGroupView = [[SocialDisplayGroupMenuView alloc] initWithFrame:self.bounds];
    }
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [self.socialDisplayGroupView reset];
        [self addSubview:self.socialDisplayGroupView];
    });
}

- (IBAction) favoritesGroupButtonTapped {
    SampleLog(LL_TRACE, "Social-Group Favorites Group tapped.");

    if (!self.socialDisplayGroupView) {
        self.socialDisplayGroupView = [[SocialDisplayGroupMenuView alloc] initWithFrame:self.bounds];
    }
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [self.socialDisplayGroupView reset];
        [self addSubview:self.socialDisplayGroupView];
    });
}

- (IBAction) backToSocialButtonTapped {
    SampleLog(LL_TRACE, "Social-Group Back-To-Social tapped.");
    
    [self backToPreviousMenu];
}

@end
