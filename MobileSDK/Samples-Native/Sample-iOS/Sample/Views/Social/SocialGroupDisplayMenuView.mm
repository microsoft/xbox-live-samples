// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import "SocialGroupDisplayMenuView.h"
#import "SocialGroupDisplayView.h"
#import "SocialGroupMenuView.h"

@interface SocialGroupDisplayMenuView()

@property (nonatomic, weak) IBOutlet UIView* contentView;
@property (nonatomic, weak) IBOutlet SocialGroupDisplayView* socialGroupDisplayView;
@property (nonatomic, weak) IBOutlet UIButton* backToSocialGroupButton;

@end

@implementation SocialGroupDisplayMenuView

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
    [[NSBundle mainBundle] loadNibNamed:@"SocialGroupDisplayMenuView" owner:self options:nil];
    [self addSubview:self.contentView];
    self.contentView.frame = self.bounds;
    
    self.backToSocialGroupButton.layer.borderWidth = 1.0f;
    self.backToSocialGroupButton.layer.borderColor = [UIColor lightGrayColor].CGColor;
    self.backToSocialGroupButton.layer.cornerRadius = 10.0f;
}

- (void)dealloc {
    SampleLog(LL_TRACE, "Social-Group-Display Menu dealloc.");
}

- (void)setSocialGroup:(XblSocialManagerUserGroup*)socialGroup {
    [self.socialGroupDisplayView setSocialGroup:socialGroup];
}

- (void)refreshSocialGroup {
    [self.socialGroupDisplayView refreshSocialGroup];
}

#pragma mark - IBActions

- (IBAction) backToSocialGroupButtonTapped {
    SampleLog(LL_TRACE, "Social-Group-Display Back-To-Social-Group tapped.");
    
    if (self.parentMenu) {
        [self.parentMenu socialGroupDisplayMenuExit];
    }
}

@end
