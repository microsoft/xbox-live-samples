// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import "SocialUserMenuView.h"
#import <Identity_Integration.h>
#import <Social_Integration.h>
#import <Game_Integration.h>
#import <SocialUserMenu_Integration.h>
#import "SocialMenuView.h"
#import "SocialUserDisplayMenuView.h"
#import "IdentityDisplayView.h"

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
@property (nonatomic, weak) IBOutlet UITableView* tableView;
@property (nonatomic, weak) IBOutlet UIButton* backToSocialButton;

@property (nonatomic, strong) SocialUserDisplayMenuView* socialDisplayUserMenuView;
@property (nonatomic, assign) NSUInteger selectedUser;

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

    self.tableView.layer.borderWidth = 1.0f;
    self.tableView.layer.borderColor = [UIColor lightGrayColor].CGColor;

    self.backToSocialButton.layer.borderWidth = 1.0f;
    self.backToSocialButton.layer.borderColor = [UIColor lightGrayColor].CGColor;
    self.backToSocialButton.layer.cornerRadius = 10.0f;

    SocialUserMenu_Integration::getInstance()->socialUserMenuInstance = (void*)CFBridgingRetain(self);
}

- (void)dealloc {
    SampleLog(LL_TRACE, "Social-User Menu dealloc.");
}

- (void)socialUserDisplayMenuExit {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (self.socialDisplayUserMenuView) {
            [self.socialDisplayUserMenuView removeFromSuperview];
            self.socialDisplayUserMenuView = nil;
        }
    });
}

- (void)updateUserImageView:(UIImage*)image {
    if (self.socialDisplayUserMenuView) {
        [self.socialDisplayUserMenuView updateUserImageView:image];
    }
}

- (void)updateUserIDLabel:(NSString*)title {
    if (self.socialDisplayUserMenuView) {
        [self.socialDisplayUserMenuView updateUserIDLabel:title];
    }
}

- (void)updateUserGamerScore:(NSString*)score {
    if (self.socialDisplayUserMenuView) {
        [self.socialDisplayUserMenuView updateUserGamerScore:score];
    }
}

- (void)updateUserStatus:(NSString*)status {
    if (self.socialDisplayUserMenuView) {
        [self.socialDisplayUserMenuView updateUserStatus:status];
    }
}

- (void)updateUserRelationship:(NSString*)relationship {
    if (self.socialDisplayUserMenuView) {
        [self.socialDisplayUserMenuView updateUserRelationship:relationship];
    }
}

#pragma mark - UITableViewDataSource delegates

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section {
    return 10;
}

- (UITableViewCell*)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    static NSString* tableCellIdentifier = @"SocialUserTableCell";

    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:tableCellIdentifier];
    if (cell == nil) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:tableCellIdentifier];
    }

    cell.textLabel.text = [NSString stringWithFormat:@"View User %ld", (long)indexPath.row];

    return cell;
}

#pragma view - UITableViewDelegate delegates

- (void)tableView:(UITableView*)tableView didSelectRowAtIndexPath:(NSIndexPath*)indexPath {
    SampleLog(LL_TRACE, "Clicked View User %d.", indexPath.row);

    self.selectedUser = indexPath.row;

    dispatch_async(dispatch_get_main_queue(), ^{
        self.socialDisplayUserMenuView = [[SocialUserDisplayMenuView alloc] initWithFrame:self.bounds];
        self.socialDisplayUserMenuView.parentMenu = self;
        [self addSubview:self.socialDisplayUserMenuView];
    });

    XblContextHandle contextHandle = Game_Integration::getInstance()->getXblContext();
    Identity_GetGamerProfileAsync(nil, contextHandle, hardCodedUserIds[self.selectedUser]);
}

#pragma mark - IBActions

- (IBAction) backToSocialTapped {
    SampleLog(LL_TRACE, "Social-User Back-To-Social tapped.");

    CFRelease(SocialUserMenu_Integration::getInstance()->socialUserMenuInstance);
    SocialUserMenu_Integration::getInstance()->socialUserMenuInstance = nullptr;

    if (self.parentMenu) {
        [self.parentMenu socialUserMenuExit];
    }
}

@end
