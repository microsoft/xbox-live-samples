//
//  IdentityMenuView.m
//  Sample
//
//  Created by Xlive XdevX on 4/2/19.
//  Copyright © 2019 Microsoft Xbox Live. All rights reserved.
//

#import "IdentityMenuView.h"
#import <Identity_Integration.h>
#import <Game_Integration.h>
#import <IdentityMenu_Integration.h>
#import <Social_Integration.h>

#define DEFAULT_ID_TITLE    @"Xbox Live iOS Sample"

@interface IdentityMenuView() {}

@property (nonatomic, weak) IBOutlet UIView *contentView;
@property (nonatomic, weak) IBOutlet UIImageView *userImageView;
@property (nonatomic, weak) IBOutlet UILabel *userIDLabel;
@property (nonatomic, weak) IBOutlet UILabel *userGamerScoreLabel;
@property (nonatomic, weak) IBOutlet UIButton *signInButton;
@property (nonatomic, weak) IBOutlet UIButton *signOutButton;

@property (nonatomic, assign) NSUInteger signInState;

@end

@implementation IdentityMenuView

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
    [[NSBundle mainBundle] loadNibNamed:@"IdentityMenuView" owner:self options:nil];
    [self addSubview:self.contentView];
    self.contentView.frame = self.bounds;

    self.signInButton.layer.borderWidth = 1.0f;
    self.signInButton.layer.borderColor = [UIColor lightGrayColor].CGColor;
    self.signInButton.layer.cornerRadius = 10.0f;
    
    self.signOutButton.layer.borderWidth = 1.0f;
    self.signOutButton.layer.borderColor = [UIColor lightGrayColor].CGColor;
    self.signOutButton.layer.cornerRadius = 10.0f;
    
    self.userImageView.layer.cornerRadius = self.userImageView.bounds.size.width / 2.0;
    
    [self updateIdentityButtons:ID_NONE];
    [self updateUserImageView:nil];
    [self updateUserIDLabel:nil];
    [self updateUserGamerScore:nil];

    IdentityMenu_Integration::getInstance()->identityMenuInstance = (void*)CFBridgingRetain(self);
}

- (void)dealloc {
    SampleLog(LL_TRACE, "Identity Menu dealloc.");
}

- (void)updateIdentityButtons:(int)status {
    dispatch_async(dispatch_get_main_queue(), ^{
        switch (status) {
            case ID_SIGNED_IN:
                self.signInButton.enabled = NO;
                self.signOutButton.enabled = YES;
                break;
            case ID_SIGNED_OUT:
                self.signInButton.enabled = YES;
                self.signOutButton.enabled = NO;
                break;
            default:
                self.signInButton.enabled = NO;
                self.signOutButton.enabled = NO;
                break;
        }
    });
    self.signInState = status;
}

-(void)updateUserAndContext:(int)status {
    switch (self.signInState) {
        case ID_SIGNED_IN: {
            // Try to get the new user's profile.
            XblContextHandle contextHandle = Game_Integration::getInstance()->getXblContext();
            Identity_GetDefaultGamerProfileAsync(nil, contextHandle);

            // Add the signed-in user to Social.
            XalUserHandle user = Game_Integration::getInstance()->getCurrentUser();
            if (user) {
                Social_AddUserToSocialManager(user);
            }

            uint64_t userId = Game_Integration::getInstance()->getCurrentUserId();
            SampleLog(LL_INFO, "Signed in user ID %llX.", userId);

            break;
        }

        case ID_SIGNED_OUT: {
            // Remove the signed-out user from Social.
            XalUserHandle user = Game_Integration::getInstance()->getCurrentUser();
            if (user) {
                uint64_t userId = Game_Integration::getInstance()->getCurrentUserId();
                SampleLog(LL_INFO, "Signed out user ID %llX.", userId);
                Social_RemoveUserFromSocialManager(user);
            }

            break;
        }

        default:
            break;
    }
    
}

- (void)updateUserImageView:(UIImage*)image {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (image == nil) {
            self.userImageView.image = [UIImage imageNamed:@"xboxIcon"];
            return;
        }
        
        self.userImageView.image = image;
    });
}

- (void)updateUserIDLabel:(NSString*)title {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (title == nil) {
            self.userIDLabel.text = DEFAULT_ID_TITLE;
            return;
        }
    
        self.userIDLabel.text = title;
    });
}

- (void)updateUserGamerScore:(NSString*)score {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (score == nil) {
            self.userGamerScoreLabel.text = @"";
            return;
        }
        
        self.userGamerScoreLabel.text = [NSString stringWithFormat:@"Gamer Score: %@", score];
    });
}

#pragma mark - IBActions

- (IBAction)signInAction {
    self.signInButton.enabled = NO;
    
    Identity_TrySignInUserWithUI(nil);
}

- (IBAction)signOutAction {
    self.signOutButton.enabled = NO;
    
    Identity_TrySignOutUser(nil, Game_Integration::getInstance()->getCurrentUser());
}

@end
