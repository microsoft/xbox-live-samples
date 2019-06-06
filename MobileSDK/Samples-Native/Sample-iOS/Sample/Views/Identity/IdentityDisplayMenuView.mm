// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import "IdentityDisplayMenuView.h"
#import <Identity_Integration.h>
#import <Game_Integration.h>
#import <IdentityDisplayMenu_Integration.h>
#import "SocialUserMenuView.h"
#import "IdentityDisplayView.h"

@interface IdentityDisplayMenuView() {}

@property (nonatomic, weak) IBOutlet UIView *contentView;
@property (nonatomic, weak) IBOutlet UIView *identityContainer;
@property (nonatomic, weak) IBOutlet UIButton *backToSocialUserButton;

@property (nonatomic, strong) IdentityDisplayView *identityDisplayView;

@end

@implementation IdentityDisplayMenuView

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
    [[NSBundle mainBundle] loadNibNamed:@"IdentityDisplayMenuView" owner:self options:nil];
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

    IdentityDisplayMenu_Integration::getInstance()->identityDisplayMenuInstance = (void*)CFBridgingRetain(self);
}

- (void)dealloc {
    SampleLog(LL_TRACE, "Social-User-Display Menu dealloc.");
}

- (void)setXboxLiveUserId:(uint64_t)userId {
    XblContextHandle contextHandle = Game_Integration::getInstance()->getXblContext();
    Identity_GetGamerProfileAsync(nil, contextHandle, userId);
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

- (void)updateUserRelationship:(NSString*)relationship {
    if (self.identityDisplayView) {
        [self.identityDisplayView updateUserRelationship:relationship];
    }
}

#pragma mark - IBActions

- (IBAction) backToSocialUserButtonTapped {
    SampleLog(LL_TRACE, "Social-User-Display Back-To-Social-User tapped.");

    CFRelease(IdentityDisplayMenu_Integration::getInstance()->identityDisplayMenuInstance);
    IdentityDisplayMenu_Integration::getInstance()->identityDisplayMenuInstance = nullptr;

    if (self.parentMenu) {
        [self.parentMenu identityDisplayMenuExit];
    }
}

@end
