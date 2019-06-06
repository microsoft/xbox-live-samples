// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import "SocialUserMenuView.h"
#import <Social_Integration.h>
#import <Game_Integration.h>
#import "SocialMenuView.h"
#import "SocialGroupDisplayView.h"
#import "IdentityDisplayMenuView.h"

// XBL User IDs for all 10 test accounts.
static uint64_t hardCodedUserIds[10] = {
    0x0009FFE3414C22A3,
    0x0009FFE964D27B1B,
    0x0009FFE1B9AF5BEF,
    0x0009FFEA7D1CC11A,
    0x0009FFE7D7FE67F6,
    0x0009FFE8E11E678B,
    0x0009FFE91F91FF9D,
    0x0009FFEE9617FA69,
    0x0009FFE033750C35,
    0x0009FFE93045D12A,
};

@interface SocialUserMenuView()

@property (nonatomic, weak) IBOutlet UIView* contentView;
@property (nonatomic, weak) IBOutlet SocialGroupDisplayView* socialGroupDisplayView;
@property (nonatomic, weak) IBOutlet UIButton* backToSocialButton;

@property (nonatomic, strong) IdentityDisplayMenuView* identityDisplayMenuView;
@property (nonatomic, assign) uint64_t selectedUser;

@end

@implementation SocialUserMenuView

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
    [[NSBundle mainBundle] loadNibNamed:@"SocialUserMenuView" owner:self options:nil];
    [self addSubview:self.contentView];
    self.contentView.frame = self.bounds;

    self.parentMenu = nil;

    self.socialGroupDisplayView.layer.borderWidth = 1.0f;
    self.socialGroupDisplayView.layer.borderColor = [UIColor lightGrayColor].CGColor;
    self.socialGroupDisplayView.socialGroupDisplayDelegate = self;

    self.backToSocialButton.layer.borderWidth = 1.0f;
    self.backToSocialButton.layer.borderColor = [UIColor lightGrayColor].CGColor;
    self.backToSocialButton.layer.cornerRadius = 10.0f;

    // Create a local social group with a fixed set of Xbox Live IDs.
    XalUserHandle user = Game_Integration::getInstance()->getCurrentUser();
    XblSocialManagerUserGroup* socialGroup = Social_CreateSocialGroupFromList(user, hardCodedUserIds, 10);
    [self.socialGroupDisplayView setSocialGroup:socialGroup];
}

- (void)dealloc {
    SampleLog(LL_TRACE, "Social-User Menu dealloc.");
}

- (void)identityDisplayMenuExit {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (self.identityDisplayMenuView) {
            [self.identityDisplayMenuView removeFromSuperview];
            self.identityDisplayMenuView = nil;
        }
    });
}

#pragma mark - SocialGroupDisplayDelegate

- (void)SocialUserTappedWithXboxId:(uint64_t)userId {
    // TEMP! Until we can get real user data, this passes in the index to the hard-coded ids.
    self.selectedUser = hardCodedUserIds[userId];

    dispatch_async(dispatch_get_main_queue(), ^{
        self.identityDisplayMenuView = [[IdentityDisplayMenuView alloc] initWithFrame:self.bounds];
        self.identityDisplayMenuView.parentMenu = self;
        [self.identityDisplayMenuView setXboxLiveUserId:self.selectedUser];
        [self addSubview:self.identityDisplayMenuView];
    });
}

#pragma mark - IBActions

- (IBAction) backToSocialTapped {
    SampleLog(LL_TRACE, "Social-User Back-To-Social tapped.");

    if (self.parentMenu) {
        [self.parentMenu socialUserMenuExit];
    }
}

@end
