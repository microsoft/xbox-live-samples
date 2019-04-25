// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import "HubMenuView.h"
#import "AchievementsMenuView.h"
#import "SocialMenuView.h"

@interface HubMenuView() {}

@property (nonatomic, weak) IBOutlet UIView *contentView;
@property (nonatomic, weak) IBOutlet UIButton *achievementButton;
@property (nonatomic, weak) IBOutlet UIButton *socialButton;

@end

@implementation HubMenuView

// NOTE: HubMenuView is instanciated by the main storyboard view controller,
// so it does not match the standard singleton pattern.
static HubMenuView* sharedInstance = nil;
+ (HubMenuView*)shared {
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
    sharedInstance = self;
    
    [[NSBundle mainBundle] loadNibNamed:@"HubMenuView" owner:self options:nil];
    [self addSubview:self.contentView];
    self.contentView.frame = self.bounds;
    
    self.achievementButton.layer.borderWidth = 1.0f;
    self.achievementButton.layer.borderColor = [UIColor lightGrayColor].CGColor;
    self.achievementButton.layer.cornerRadius = 10.0f;
    
    self.socialButton.layer.borderWidth = 1.0f;
    self.socialButton.layer.borderColor = [UIColor lightGrayColor].CGColor;
    self.socialButton.layer.cornerRadius = 10.0f;
}

- (void)updateMenuHidden:(BOOL)hidden {
    if (hidden) {
        [[AchievementsMenuView shared] backToPreviousMenu];
        [[SocialMenuView shared] updateMenuHidden:hidden];
        [[SocialMenuView shared] backToPreviousMenu];
    }
    
    dispatch_async(dispatch_get_main_queue(), ^{
        self.hidden = hidden;
    });
}

- (IBAction)achievementsButtonTapped {
    SampleLog(LL_TRACE, "Hub menu Achievements tapped.");
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [AchievementsMenuView shared].frame = self.bounds;
        [[AchievementsMenuView shared] reset];
        [self addSubview:[AchievementsMenuView shared]];
    });
}

- (IBAction)socialButtonTapped {
    SampleLog(LL_TRACE, "Hub menu Social tapped.");

    dispatch_async(dispatch_get_main_queue(), ^{
        [SocialMenuView shared].frame = self.bounds;
        [[SocialMenuView shared] reset];
        [self addSubview:[SocialMenuView shared]];
    });
}

@end
