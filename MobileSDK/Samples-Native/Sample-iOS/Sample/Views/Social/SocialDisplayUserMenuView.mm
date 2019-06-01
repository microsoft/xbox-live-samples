// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import "SocialUserDisplayMenuView.h"
#import "IdentityDisplayView.h"
#import "SocialUserMenuView.h"

@interface SocialUserDisplayMenuView() {}

@property (nonatomic, weak) IBOutlet UIView *contentView;
@property (nonatomic, weak) IBOutlet UIView *identityContainer;
@property (nonatomic, weak) IBOutlet UIButton *backToSocialUserButton;

@property (nonatomic, strong) IdentityDisplayView *identityDisplayView;

@end

@implementation SocialUserDisplayMenuView

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

    self.backToSocialUserButton.layer.borderWidth = 1.0f;
    self.backToSocialUserButton.layer.borderColor = [UIColor lightGrayColor].CGColor;
    self.backToSocialUserButton.layer.cornerRadius = 10.0f;

    self.identityDisplayView = [[IdentityDisplayView alloc] initWithFrame:CGRectZero];
    [self.identityDisplayView embedInView:self.identityContainer];

    [self updateUserIDLabel:nil];
    [self updateUserImageView:nil];
    [self updateUserGamerScore:nil];
    [self updateUserStatus:nil];
    [self updateUserRelationship:nil];
}

- (void)dealloc {
    SampleLog(LL_TRACE, "Social-Display-User Menu dealloc.");
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

- (void)updateUserStatus:(NSString*)status {
    if (self.identityDisplayView) {
        [self.identityDisplayView updateUserStatus:status];
    }
}

- (void)updateUserRelationship:(NSString *)relationship {
    if (self.identityDisplayView) {
        [self.identityDisplayView updateUserRelationship:relationship];
    }
}

#pragma mark - IBActions

- (IBAction) backToSocialUserButtonTapped {
    SampleLog(LL_TRACE, "Social-Display-User Back-To-Social-User tapped.");

    if (self.parentMenu) {
        [self.parentMenu socialUserDisplayMenuExit];
    }
}

@end
