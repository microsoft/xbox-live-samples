// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import "SocialGroupMenuView.h"
#import <GameScene.h>
#import "SocialDisplayGroupMenuView.h"

@interface SocialGroupMenuView() {
    XblSocialManagerUserGroup* socialGroupFriends;
    XblSocialManagerUserGroup* socialGroupFavorite;
}

@property (nonatomic, weak) IBOutlet UIView *contentView;
@property (nonatomic, weak) IBOutlet UIButton *friendsGroupButton;
@property (nonatomic, weak) IBOutlet UIButton *favoritesGroupButton;
@property (nonatomic, weak) IBOutlet UIButton *backToSocialButton;

@end

@implementation SocialGroupMenuView

static SocialGroupMenuView *sharedInstance = [[SocialGroupMenuView alloc] initWithFrame:CGRectZero];
+ (SocialGroupMenuView*)shared {
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
    self->socialGroupFriends = nil;
    self->socialGroupFavorite = nil;

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
}

- (void)dealloc {
    SampleLog(LL_TRACE, "Social-Group Menu dealloc!!!!");
}

- (void)reset {
}

- (void)updateMenuHidden:(BOOL)hidden {
    if (hidden) {
        [[SocialDisplayGroupMenuView shared] backToPreviousMenu];
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

- (void)setSocialGroupFriends:(XblSocialManagerUserGroup*)friends {
    self->socialGroupFriends = friends;

    dispatch_async(dispatch_get_main_queue(), ^{
        if (self->socialGroupFriends) {
            self.friendsGroupButton.enabled = self->socialGroupFriends->usersCount > 0;
        } else {
            self.friendsGroupButton.enabled = false;
        }
    });
}

- (void)setSocialGroupFavorites:(XblSocialManagerUserGroup*)favorites {
    self->socialGroupFavorite = favorites;

    dispatch_async(dispatch_get_main_queue(), ^{
        if (self->socialGroupFavorite) {
            self.favoritesGroupButton.enabled = self->socialGroupFavorite->usersCount > 0;
        } else {
            self.favoritesGroupButton.enabled = false;
        }
    });
}

- (void)refreshSocialGroups {
    [[SocialDisplayGroupMenuView shared] refreshSocialGroup];
}

#pragma mark - IBActions

- (IBAction) friendsGroupButtonTapped {
    SampleLog(LL_TRACE, "Social-Group Friends Group tapped.");

    dispatch_async(dispatch_get_main_queue(), ^{
        [SocialDisplayGroupMenuView shared].frame = self.bounds;
        [[SocialDisplayGroupMenuView shared] reset];
        [[SocialDisplayGroupMenuView shared] setSocialGroup:self->socialGroupFriends];
        [self addSubview:[SocialDisplayGroupMenuView shared]];
    });
}

- (IBAction) favoritesGroupButtonTapped {
    SampleLog(LL_TRACE, "Social-Group Favorites Group tapped.");

    dispatch_async(dispatch_get_main_queue(), ^{
        [SocialDisplayGroupMenuView shared].frame = self.bounds;
        [[SocialDisplayGroupMenuView shared] reset];
        [[SocialDisplayGroupMenuView shared] setSocialGroup:self->socialGroupFavorite];
        [self addSubview:[SocialDisplayGroupMenuView shared]];
    });
}

- (IBAction) backToSocialButtonTapped {
    SampleLog(LL_TRACE, "Social-Group Back-To-Social tapped.");
    
    [self backToPreviousMenu];
}

@end
