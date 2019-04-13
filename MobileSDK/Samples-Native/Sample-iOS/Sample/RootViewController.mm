// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import "RootViewController.h"
#import "TextViewLog.h"
#import <Identity_Integration.h>
#import <XSAPI_Integration.h>
#import <GameScene.h>

@interface RootViewController ()

@property (nonatomic, weak) IBOutlet UITextView *logTextView;
@property (nonatomic, weak) IBOutlet UIView *menuContainerView;

@end

@implementation RootViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.logTextView.text = @"";
    self.logTextView.layoutManager.allowsNonContiguousLayout = false;
    [[TextViewLog shared] setTextView:self.logTextView];
    
    // Init Xbox Live services.
    GameScene::getInstance()->xboxLive_init();
    
    // Try auto-sign-in.
    HRESULT hr = Identity_TrySignInUserSilently(XblGetAsyncQueue());
    if (FAILED(hr))
    {
        SampleLog(LL_ERROR, "XalTryAddDefaultUserSilentlyAsync Failed!");
        SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(hr).c_str());
    }
}

@end
