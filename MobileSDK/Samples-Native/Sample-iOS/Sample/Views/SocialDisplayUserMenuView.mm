// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import "SocialDisplayUserMenuView.h"
#import "IdentityDisplayView.h"

@interface SocialDisplayUserMenuView() {}

@property (nonatomic, weak) IBOutlet UIView *contentView;
@property (nonatomic, weak) IBOutlet UIView *identityContainer;
@property (nonatomic, weak) IBOutlet UIButton *changeRelationshipButton;
@property (nonatomic, weak) IBOutlet UIButton *backToSocialUserButton;

@property (nonatomic, strong) IdentityDisplayView *identityDisplayView;

@end

@implementation SocialDisplayUserMenuView

+ (SocialDisplayUserMenuView*)shared {
    static SocialDisplayUserMenuView *sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[SocialDisplayUserMenuView alloc] initWithFrame:CGRectZero];
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
    [[NSBundle mainBundle] loadNibNamed:@"SocialDisplayUserMenuView" owner:self options:nil];
    [self addSubview:self.contentView];
    self.contentView.frame = self.bounds;

    self.changeRelationshipButton.layer.borderWidth = 1.0f;
    self.changeRelationshipButton.layer.borderColor = [UIColor lightGrayColor].CGColor;
    self.changeRelationshipButton.layer.cornerRadius = 10.0f;

    self.backToSocialUserButton.layer.borderWidth = 1.0f;
    self.backToSocialUserButton.layer.borderColor = [UIColor lightGrayColor].CGColor;
    self.backToSocialUserButton.layer.cornerRadius = 10.0f;

    self.identityDisplayView = [[IdentityDisplayView alloc] initWithFrame:CGRectZero];
    [self.identityDisplayView embedInView:self.identityContainer];
    
    [self reset];

    //AchievementsMenu_Integration::getInstance()->achievementsMenuInstance = (void *)CFBridgingRetain(self);
}

- (void)dealloc {
    SampleLog(LL_TRACE, "Social-Display-User Menu dealloc!!!!");
}

- (void)reset {
}

- (void)backToPreviousMenu {
    dispatch_async(dispatch_get_main_queue(), ^{
        [self removeFromSuperview];
    });
}

- (void)updateUserImageView:(UIImage*)image {
    if (self.identityDisplayView) {
        [self.identityDisplayView updateUserImageView:image];
    }
}

- (void)updateUserIDLabel:(NSString*)title {
    if (self.identityDisplayView) {
        [self.identityDisplayView updateUserIDLabel:title];
    }
}

- (void)updateUserGamerScore:(NSString*)score {
    if (self.identityDisplayView) {
        [self.identityDisplayView updateUserGamerScore:score];
    }
}

- (void)updateUserRelationship:(NSString *)relationship {
    if (self.identityDisplayView) {
        [self.identityDisplayView updateUserRelationship:relationship];
    }
}

#pragma mark - IBActions

- (IBAction) changeRelationshipButtonTapped {
    SampleLog(LL_TRACE, "Social-Display-User Add GroupChange Relationship tapped.");
}

- (IBAction) backToSocialUserButtonTapped {
    SampleLog(LL_TRACE, "Social-Display-User Back-To-Social-User tapped.");

    [self backToPreviousMenu];
}

@end
