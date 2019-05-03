// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import "SocialDisplayGroupMenuView.h"
#import "IdentityDisplayView.h"

@interface SocialDisplayGroupMenuView() {}

@property (nonatomic, weak) IBOutlet UIView *contentView;
@property (nonatomic, weak) IBOutlet UITableView *tableView;
@property (nonatomic, weak) IBOutlet UIButton *backToSocialGroupButton;

@end

@implementation SocialDisplayGroupMenuView

+ (SocialDisplayGroupMenuView*)shared {
    static SocialDisplayGroupMenuView *sharedInstance = [[SocialDisplayGroupMenuView alloc] initWithFrame:CGRectZero];
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

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section {
    return 10;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    static NSString *tableCellIdentifier = @"SocialDisplayGroupTableCell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:tableCellIdentifier];
    if (cell == nil) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:tableCellIdentifier];
        
        IdentityDisplayView *identityDisplayView = [[IdentityDisplayView alloc] initWithFrame:CGRectZero];
        [identityDisplayView embedInView:cell];
    }
    
    return cell;
}

#pragma mark - IBActions

- (IBAction) backToSocialGroupButtonTapped {
    SampleLog(LL_TRACE, "Social-Display-Group Back-To-Social-Group tapped.");
    
    [self backToPreviousMenu];
}

@end
