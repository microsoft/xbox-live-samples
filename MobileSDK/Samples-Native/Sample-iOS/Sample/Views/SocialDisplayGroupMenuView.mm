// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import "SocialDisplayGroupMenuView.h"
#import "IdentityDisplayView.h"
#import "SocialGroupDisplayTableViewCell.h"

@interface SocialDisplayGroupMenuView() {
    XblSocialManagerUserGroup* socialGroup;
    uint32_t socialGroupSize;
    XblSocialManagerUser socialGroupUsers[64];
}

@property (nonatomic, weak) IBOutlet UIView *contentView;
@property (nonatomic, weak) IBOutlet UITableView *tableView;
@property (nonatomic, weak) IBOutlet UIButton *backToSocialGroupButton;

@end

@implementation SocialDisplayGroupMenuView

static SocialDisplayGroupMenuView *sharedInstance = [[SocialDisplayGroupMenuView alloc] initWithFrame:CGRectZero];
+ (SocialDisplayGroupMenuView*)shared {
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
    [[NSBundle mainBundle] loadNibNamed:@"SocialDisplayGroupMenuView" owner:self options:nil];
    [self addSubview:self.contentView];
    self.contentView.frame = self.bounds;
    
    self.tableView.layer.borderWidth = 1.0f;
    self.tableView.layer.borderColor = [UIColor lightGrayColor].CGColor;

    self.backToSocialGroupButton.layer.borderWidth = 1.0f;
    self.backToSocialGroupButton.layer.borderColor = [UIColor lightGrayColor].CGColor;
    self.backToSocialGroupButton.layer.cornerRadius = 10.0f;

    [self reset];
}

- (void)reset {
}

- (void)backToPreviousMenu {
    dispatch_async(dispatch_get_main_queue(), ^{
        [self removeFromSuperview];
    });
}

- (void)setSocialGroup:(XblSocialManagerUserGroup*)socialGroup {
    self->socialGroup = socialGroup;
    self->socialGroupSize = 0;

    [self refreshSocialGroup];
}

- (void)refreshSocialGroup {
    if (self->socialGroup) {
        self->socialGroupSize = self->socialGroup->usersCount;

        XblSocialManagerUserGroupGetUsers(self->socialGroup, self->socialGroupSize, self->socialGroupUsers);

        dispatch_async(dispatch_get_main_queue(), ^{
            [self.tableView reloadData];
        });
    }
}

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section {
    return self->socialGroupSize;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    static NSString *tableCellIdentifier = @"SocialDisplayGroupTableCell";
    
    SocialGroupDisplayTableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:tableCellIdentifier];
    if (cell == nil) {
        cell = [[SocialGroupDisplayTableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:tableCellIdentifier];
        
        cell.identityDisplayView = [[IdentityDisplayView alloc] initWithFrame:CGRectZero];
        [cell.identityDisplayView embedInView:cell];
    }

    if (indexPath.row < self->socialGroupSize) {
        XblSocialManagerUser user = self->socialGroupUsers[indexPath.row];

        [cell.identityDisplayView updateUserIDLabel:[NSString stringWithUTF8String:user.gamertag]];

        NSString* urlString = [NSString stringWithUTF8String:user.displayPicUrlRaw];
        UIImage* userImage = [UIImage imageWithData:[NSData dataWithContentsOfURL:[NSURL URLWithString:urlString]]];
        [cell.identityDisplayView updateUserImageView:userImage];

        [cell.identityDisplayView updateUserGamerScore:[NSString stringWithUTF8String:user.gamerscore]];
        [cell.identityDisplayView updateUserRelationship:user.isFavorite ? @"Favorite" : @"Friend"];

        /* Not finding the Xbl Presence header files???
        NSString* userState;
        switch (user.presenceRecord.userState) {
            case XblPresenceUserState.Online:
                userState = @"Online";
                break;
            case XblPresenceUserState.Away:
                userState = @"Away";
                break;
            case XblPresenceUserState.Offline:
                userState = @"Offline";
                break;
            //case XblPresenceUserState.Unknown:
            default:
                userState = @"Unknown";
                break;
        }
        [cell.identityDisplayView updateUserStatus:userState];
         */ [cell.identityDisplayView updateUserStatus:nil];
    } else {
        [cell.identityDisplayView updateUserIDLabel:nil];
        [cell.identityDisplayView updateUserImageView:nil];
        [cell.identityDisplayView updateUserGamerScore:nil];
        [cell.identityDisplayView updateUserRelationship:nil];
        [cell.identityDisplayView updateUserStatus:nil];
    }
    return cell;
}

#pragma mark - IBActions

- (IBAction) backToSocialGroupButtonTapped {
    SampleLog(LL_TRACE, "Social-Display-Group Back-To-Social-Group tapped.");
    
    [self backToPreviousMenu];
}

@end
