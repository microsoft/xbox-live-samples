// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import "IdentityMenuView.h"
#import <Identity_Integration.h>
#import <GameScene.h>
#import <IdentityMenu_Integration.h>

#define DEFAULT_ID_TITLE    @"Xbox Live iOS Sample"

@interface IdentityMenuView() {}

@property (nonatomic, weak) IBOutlet UIView *contentView;
@property (nonatomic, weak) IBOutlet UIImageView *userImageView;
@property (nonatomic, weak) IBOutlet UILabel *userIDLabel;
@property (nonatomic, weak) IBOutlet UILabel *userGamerScoreLabel;
@property (nonatomic, weak) IBOutlet UIButton *signInButton;
@property (nonatomic, weak) IBOutlet UIButton *signOutButton;

@end

@implementation IdentityMenuView

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
    
    IdentityMenu_Integration::getInstance()->identityMenuInstance = (void *)CFBridgingRetain(self);
    [self updateIdentityButtons:ID_NONE];
    [self updateUserImageView:nil];
    [self updateUserIDLabel:nil];
    [self updateUserGamerScore:nil];
}

- (void)dealloc {
    SampleLog(LL_TRACE, "Identity Menu dealloc!!!!");
}

- (void)updateIdentityButtons:(int)status {
    dispatch_async(dispatch_get_main_queue(), ^{
        switch (status) {
            case ID_SIGNED_IN:
                self.signInButton.enabled = false;
                self.signOutButton.enabled = true;
                break;
            case ID_SIGNED_OUT:
                self.signInButton.enabled = true;
                self.signOutButton.enabled = false;
                break;
            default:
                self.signInButton.enabled = false;
                self.signOutButton.enabled = false;
                break;
        }
    });
    
    if (status == ID_SIGNED_IN) {
        // Try to grab the user's profile.
        XblContextHandle contextHandle = GameScene::getInstance()->getXblContext();
        Identity_GetDefaultGamerProfileAsync(nil, contextHandle);
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
    self.signInButton.enabled = false;
    
    Identity_TrySignInUserWithUI(nil);
}

- (IBAction)signOutAction {
    self.signOutButton.enabled = false;
    
    Identity_TrySignOutUser(nil, GameScene::getInstance()->getCurrentUser());
}

@end
