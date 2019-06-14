//
//  SocialGroupDisplayView.m
//  Sample
//
//  Created by Douglas Lanford on 5/29/19.
//  Copyright © 2019 Microsoft Xbox Live. All rights reserved.
//

#import "SocialGroupDisplayView.h"
#import "IdentityDisplayView.h"
#import "SocialGroupDisplayTableViewCell.h"

@interface SocialGroupDisplayView() {
    XblSocialManagerUserGroup* socialGroup;
    uint32_t socialGroupSize;
    XblSocialManagerUser socialGroupUsers[32];
    uint64_t socialGroupTrackedIds[32];
}

@property (nonatomic, strong) UILabel* emptyUsersPlaceholder;

@end

@implementation SocialGroupDisplayView

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
    self.delegate = self;
    self.dataSource = self;
    
    self.layer.borderWidth = 1.0f;
    self.layer.borderColor = [UIColor lightGrayColor].CGColor;
    self.tableFooterView = [[UIView alloc] initWithFrame:CGRectZero];

    self.emptyUsersPlaceholder = [[UILabel alloc] init];
    self.emptyUsersPlaceholder.font = [UIFont systemFontOfSize:17.0];
    self.emptyUsersPlaceholder.textColor = [UIColor darkGrayColor];
    self.emptyUsersPlaceholder.textAlignment = NSTextAlignmentCenter;
    self.emptyUsersPlaceholder.text = @"No Users to Display";
    self.emptyUsersPlaceholder.hidden = YES;
    self.emptyUsersPlaceholder.translatesAutoresizingMaskIntoConstraints = NO;
    [self addSubview:self.emptyUsersPlaceholder];

    NSDictionary *views = @{@"subview" : self.emptyUsersPlaceholder};
    [self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"H:|-20-[subview]-20-|" options:0 metrics:nil views:views]];
    [self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|-20-[subview(20)]" options:0 metrics:nil views:views]];
}

- (void)dealloc {
    SampleLog(LL_TRACE, "Social-Group-Display Menu dealloc.");
}

- (void)setSocialGroup:(XblSocialManagerUserGroup*)socialGroup {
    self->socialGroup = socialGroup;
    self->socialGroupSize = 0;

    [self refreshSocialGroup];
}

- (void)refreshSocialGroup {
    if (!self->socialGroup) {
        dispatch_async(dispatch_get_main_queue(), ^{
            self.emptyUsersPlaceholder.hidden = NO;
        });

        return;
    }

    uint32_t userCount = 0;
    if (self->socialGroup->socialUserGroupType == XblSocialUserGroupType::UserListType) {
        userCount = self->socialGroup->trackedUsersCount;
    } else {
        userCount = self->socialGroup->usersCount;
    }

    self->socialGroupSize = userCount;
    if (self->socialGroupSize <= 0) {
        dispatch_async(dispatch_get_main_queue(), ^{
            self.emptyUsersPlaceholder.hidden = NO;
        });

        return;
    }

    if (self->socialGroup->socialUserGroupType == XblSocialUserGroupType::UserListType) {
        // Get the user ID list for tracked users social group.
        XblSocialManagerUserGroupGetUsersTrackedByGroup(self->socialGroup, self->socialGroupSize, self->socialGroupTrackedIds);
    } else {
        // Get the users for a filtered social group.
        XblSocialManagerUserGroupGetUsers(self->socialGroup, self->socialGroupSize, self->socialGroupUsers);
    }

    dispatch_async(dispatch_get_main_queue(), ^{
        self.emptyUsersPlaceholder.hidden = YES;
    });
}

#pragma mark - UITableViewDataSource

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section {
    return self->socialGroupSize;
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    static NSString *tableCellIdentifier = @"SocialGroupDisplayTableCell";

    SocialGroupDisplayTableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:tableCellIdentifier];
    if (cell == nil) {
        cell = [[SocialGroupDisplayTableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:tableCellIdentifier];

        cell.identityDisplayView = [[IdentityDisplayView alloc] initWithFrame:CGRectZero];
        [cell.identityDisplayView embedInView:cell];
    }

    if (indexPath.row < self->socialGroupSize) {
        if (self->socialGroup->socialUserGroupType == XblSocialUserGroupType::UserListType) {
            uint64_t userId = self->socialGroupTrackedIds[indexPath.row];
            [cell.identityDisplayView updateUserIDLabel:[NSString stringWithFormat:@"ID: %llul", userId]];
        } else {
            XblSocialManagerUser user = self->socialGroupUsers[indexPath.row];

            [cell.identityDisplayView updateUserIDLabel:[NSString stringWithUTF8String:user.gamertag]];

            NSString* urlString = [NSString stringWithUTF8String:user.displayPicUrlRaw];
            UIImage* userImage = [UIImage imageWithData:[NSData dataWithContentsOfURL:[NSURL URLWithString:urlString]]];
            [cell.identityDisplayView updateUserImageView:userImage];

            [cell.identityDisplayView updateUserGamerScore:[NSString stringWithUTF8String:user.gamerscore]];
            [cell.identityDisplayView updateUserRelationship:user.isFavorite ? @"Favorite" : @"Friend"];

            // Not finding the Xbl Presence header files???
            /*
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
        }
    } else {
        [cell.identityDisplayView updateUserIDLabel:nil];
        [cell.identityDisplayView updateUserImageView:nil];
        [cell.identityDisplayView updateUserGamerScore:nil];
        [cell.identityDisplayView updateUserRelationship:nil];
        [cell.identityDisplayView updateUserStatus:nil];
    }
    return cell;
}

#pragma view - UITableViewDelegate delegates

- (void)tableView:(UITableView*)tableView didSelectRowAtIndexPath:(NSIndexPath*)indexPath {
    SampleLog(LL_TRACE, "Clicked Social Group User %d.", indexPath.row);

    if (self->socialGroup->socialUserGroupType == XblSocialUserGroupType::UserListType
        && self.socialGroupDisplayDelegate != nil) {
        uint64_t userId = self->socialGroupTrackedIds[indexPath.row];
        [self.socialGroupDisplayDelegate SocialUserTappedWithXboxId:userId];
    }
}

@end
