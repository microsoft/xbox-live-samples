// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import "AchievementsMenuView.h"
#import <Achievements_Integration.h>
#import <Game_Integration.h>
#import <AchievementsMenu_Integration.h>
#import "HubMenuView.h"

#define ACHIEVEMENT_ID_1                "1"
#define SKIP_ITEMS                      0
#define MAX_ITEMS                       3

@interface AchievementsMenuView() {}

@property (nonatomic, weak) IBOutlet UIView* contentView;
@property (nonatomic, weak) IBOutlet UIButton* achievementsButton;
@property (nonatomic, weak) IBOutlet UIButton* nextPageButton;
@property (nonatomic, weak) IBOutlet UIButton* firstAchievementButton;
@property (nonatomic, weak) IBOutlet UIButton* updateFirstButton;
@property (nonatomic, weak) IBOutlet UIButton* backToMainButton;

@property (nonatomic, assign) BOOL nextResultsPage;
@property (nonatomic, assign) XblAchievementsResultHandle resultHandle;

@end

@implementation AchievementsMenuView

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
    [[NSBundle mainBundle] loadNibNamed:@"AchievementsMenuView" owner:self options:nil];
    [self addSubview:self.contentView];
    self.contentView.frame = self.bounds;

    self.parentMenu = nil;
    
    self.achievementsButton.layer.borderWidth = 1.0f;
    self.achievementsButton.layer.borderColor = [UIColor lightGrayColor].CGColor;
    self.achievementsButton.layer.cornerRadius = 10.0f;

    self.nextPageButton.layer.borderWidth = 1.0f;
    self.nextPageButton.layer.borderColor = [UIColor lightGrayColor].CGColor;
    self.nextPageButton.layer.cornerRadius = 10.0f;

    self.firstAchievementButton.layer.borderWidth = 1.0f;
    self.firstAchievementButton.layer.borderColor = [UIColor lightGrayColor].CGColor;
    self.firstAchievementButton.layer.cornerRadius = 10.0f;

    self.updateFirstButton.layer.borderWidth = 1.0f;
    self.updateFirstButton.layer.borderColor = [UIColor lightGrayColor].CGColor;
    self.updateFirstButton.layer.cornerRadius = 10.0f;

    self.backToMainButton.layer.borderWidth = 1.0f;
    self.backToMainButton.layer.borderColor = [UIColor lightGrayColor].CGColor;
    self.backToMainButton.layer.cornerRadius = 10.0f;
    
    self.hasNextResultsPage = NO;
    self.achievementsResultHandle = nil;
    self.nextPageButton.enabled = NO;

    AchievementsMenu_Integration::getInstance()->achievementsMenuInstance = (void*)CFBridgingRetain(self);
}

- (void)dealloc {
    SampleLog(LL_TRACE, "Achievements Menu dealloc.");
}

- (void)setHasNextResultsPage:(BOOL)value {
    self.nextResultsPage = value;
    
    dispatch_async(dispatch_get_main_queue(), ^{
        self.nextPageButton.enabled = value;
    });
}

- (BOOL)getHasNextResultsPage {
    return self.nextResultsPage;
}

- (void)setAchievementsResultHandle:(XblAchievementsResultHandle)resultHandle {
    self.resultHandle = resultHandle;
}

- (XblAchievementsResultHandle)getAchievementsResultHandle {
    return self.resultHandle;
}

#pragma mark - IBActions

- (IBAction) achievementsButtonTapped {
    SampleLog(LL_TRACE, "Achievements get all achievements tapped.");
    
    if (Game_Integration::getInstance()->hasXblContext())
    {
        XblContextHandle xblContext = Game_Integration::getInstance()->getXblContext();
        XalUserHandle xalUser = Game_Integration::getInstance()->getCurrentUser();
        Achievements_GetAchievementsForTitle(XblGetAsyncQueue(), xblContext, xalUser, SKIP_ITEMS, MAX_ITEMS);
    }
}

- (IBAction) nextPageButtonTapped {
    SampleLog(LL_TRACE, "Achievements next page tapped.");
    
    if (Game_Integration::getInstance()->hasXblContext() && self.nextResultsPage)
    {
        XblContextHandle xblContext = Game_Integration::getInstance()->getXblContext();
        Achievements_GetNextResultsPage(XblGetAsyncQueue(), xblContext, self.resultHandle, MAX_ITEMS);
    }
}

- (IBAction) firstAchievementButtonTapped {
    SampleLog(LL_TRACE, "Achievements first achievement tapped.");
    
    if (Game_Integration::getInstance()->hasXblContext())
    {
        XblContextHandle xblContext = Game_Integration::getInstance()->getXblContext();
        XalUserHandle xalUser = Game_Integration::getInstance()->getCurrentUser();
        Achievements_GetAchievement(XblGetAsyncQueue(), xblContext, xalUser, ACHIEVEMENT_ID_1);
    }
}

- (IBAction) updateFirstButtonTapped {
    SampleLog(LL_TRACE, "Achievements update first achievement tapped.");
    
    if (Game_Integration::getInstance()->hasXblContext())
    {
        XblContextHandle xblContext = Game_Integration::getInstance()->getXblContext();
        XalUserHandle xalUser = Game_Integration::getInstance()->getCurrentUser();
        Achievements_UpdateAchievement(XblGetAsyncQueue(), xblContext, xalUser, ACHIEVEMENT_ID_1, 100);
    }
}

- (IBAction) backToMainButtonTapped {
    SampleLog(LL_TRACE, "Achievements Back-To-Hub tapped.");

    CFRelease(AchievementsMenu_Integration::getInstance()->achievementsMenuInstance);
    AchievementsMenu_Integration::getInstance()->achievementsMenuInstance = nullptr;

    if (self.parentMenu) {
        [self.parentMenu achievementsMenuExit];
    }
}

@end
