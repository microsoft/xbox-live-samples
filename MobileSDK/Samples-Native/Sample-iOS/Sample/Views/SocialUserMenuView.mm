// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import "SocialUserMenuView.h"
#import "SocialDisplayUserMenuView.h"

@interface SocialUserMenuView() {}

@property (nonatomic, weak) IBOutlet UIView *contentView;
@property (nonatomic, weak) IBOutlet UITableView *tableView;
@property (nonatomic, weak) IBOutlet UIButton *backToSocialButton;

@end

@implementation SocialUserMenuView

+ (SocialUserMenuView*)shared {
    static SocialUserMenuView *sharedInstance = [[SocialUserMenuView alloc] initWithFrame:CGRectZero];
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
    [[NSBundle mainBundle] loadNibNamed:@"SocialUserMenuView" owner:self options:nil];
    [self addSubview:self.contentView];
    self.contentView.frame = self.bounds;
    
    self.tableView.layer.borderWidth = 1.0f;
    self.tableView.layer.borderColor = [UIColor lightGrayColor].CGColor;

    self.backToSocialButton.layer.borderWidth = 1.0f;
    self.backToSocialButton.layer.borderColor = [UIColor lightGrayColor].CGColor;
    self.backToSocialButton.layer.cornerRadius = 10.0f;

    [self reset];
}

- (void)reset {
}

- (void)updateMenuHidden:(BOOL)hidden {
    if (hidden) {
        [[SocialDisplayUserMenuView shared] backToPreviousMenu];
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

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section {
    return 10;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    static NSString *tableCellIdentifier = @"SocialUserTableCell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:tableCellIdentifier];
    if (cell == nil) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:tableCellIdentifier];
    }
    
    cell.textLabel.text = [NSString stringWithFormat:@"View User %ld", (long)indexPath.row];
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    SampleLog(LL_TRACE, "Clicked View User.");
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [SocialDisplayUserMenuView shared].frame = self.bounds;
        [[SocialDisplayUserMenuView shared] reset];
        [self addSubview:[SocialDisplayUserMenuView shared]];
    });
}

#pragma mark - IBActions

- (IBAction) backToSocialTapped {
    SampleLog(LL_TRACE, "Social-User Back-To-Social tapped.");
    
    [self backToPreviousMenu];
}

@end
