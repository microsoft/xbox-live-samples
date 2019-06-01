// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import "SocialGroupMenuView.h"
#import <Game_Integration.h>
#import <SocialGroupMenu_Integration.h>
#import "SocialMenuView.h"
#import "SocialGroupDisplayMenuView.h"

@interface SocialGroupMenuView() {
    XblSocialManagerUserGroup* socialGroupFriends;
    XblSocialManagerUserGroup* socialGroupFavorite;
}

@property (nonatomic, weak) IBOutlet UIView* contentView;
@property (nonatomic, weak) IBOutlet UIButton* friendsGroupButton;
@property (nonatomic, weak) IBOutlet UIButton* favoritesGroupButton;
@property (nonatomic, weak) IBOutlet UIButton* backToSocialButton;

@property (strong) SocialGroupDisplayMenuView* socialGroupDisplayMenuView;

@end

@implementation SocialGroupMenuView

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

    SocialGroupMenu_Integration::getInstance()->socialGroupMenuInstance = (void*)CFBridgingRetain(self);

    [self setSocialGroupFriends:SocialGroupMenu_Integration::getInstance()->socialGroupFriends];
    [self setSocialGroupFavorites:SocialGroupMenu_Integration::getInstance()->socialGroupFavorite];
}

- (void)dealloc {
    SampleLog(LL_TRACE, "Social-Group Menu dealloc.");
}

- (void)socialGroupDisplayMenuExit {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (self.socialGroupDisplayMenuView) {
            [self.socialGroupDisplayMenuView removeFromSuperview];
            self.socialGroupDisplayMenuView = nil;
        }
    });
}

- (void)setSocialGroupFriends:(XblSocialManagerUserGroup*)friends {
    self->socialGroupFriends = friends;
}

- (void)setSocialGroupFavorites:(XblSocialManagerUserGroup*)favorites {
    self->socialGroupFavorite = favorites;
}

- (void)refreshSocialGroups {
    if (self.socialGroupDisplayMenuView) {
        [self.socialGroupDisplayMenuView refreshSocialGroup];
    }
}

#pragma mark - IBActions

- (IBAction) friendsGroupButtonTapped {
    SampleLog(LL_TRACE, "Social-Group Friends Group tapped.");

    if (!self.socialGroupDisplayMenuView) {
        dispatch_async(dispatch_get_main_queue(), ^{
            self.socialGroupDisplayMenuView = [[SocialGroupDisplayMenuView alloc] initWithFrame:self.bounds];
            self.socialGroupDisplayMenuView.parentMenu = self;
            [self.socialGroupDisplayMenuView setSocialGroup:self->socialGroupFriends];
            [self addSubview:self.socialGroupDisplayMenuView];
        });
    }
}

- (IBAction) favoritesGroupButtonTapped {
    SampleLog(LL_TRACE, "Social-Group Favorites Group tapped.");

    if (!self.socialGroupDisplayMenuView) {
        dispatch_async(dispatch_get_main_queue(), ^{
            self.socialGroupDisplayMenuView = [[SocialGroupDisplayMenuView alloc] initWithFrame:self.bounds];
            self.socialGroupDisplayMenuView.parentMenu = self;
            [self.socialGroupDisplayMenuView setSocialGroup:self->socialGroupFavorite];
            [self addSubview:self.socialGroupDisplayMenuView];
        });
    }
}

- (IBAction) backToSocialButtonTapped {
    SampleLog(LL_TRACE, "Social-Group Back-To-Social tapped.");

    CFRelease(SocialGroupMenu_Integration::getInstance()->socialGroupMenuInstance);
    SocialGroupMenu_Integration::getInstance()->socialGroupMenuInstance = nullptr;

    if (self.parentMenu) {
        [self.parentMenu socialGroupMenuExit];
    }
}

@end
