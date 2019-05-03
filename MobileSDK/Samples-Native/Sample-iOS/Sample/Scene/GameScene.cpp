// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import "GameScene.h"
#import <XSAPI_Integration.h>
#import "IdentityMenu_Integration.h"

#define SC_ID       "ad560100-4bff-49c2-a4d6-44c431374b88"
#define CLIENT_ID   "000000004824156C"
#define REDIR_URI   "ms-xal-000000004824156C://auth"
#define TITLE_ID    825707400ull
#define SANDBOX     "XDKS.1"

static GameScene* s_gameScene = nullptr;

GameScene* GameScene::getInstance()
{
    if (!s_gameScene)
    {
        s_gameScene = new (std::nothrow) GameScene();
        s_gameScene->init();
    }
    
    return s_gameScene;
}

void GameScene::init()
{
    SampleLog(LL_TRACE, "Entering Game Scene");
}

bool GameScene::xboxLive_init()
{
    // Set Debugger Trace Level for XSAPI
    HCSettingsSetTraceLevel(HCTraceLevel::Verbose);
    HCTraceSetTraceToDebugger(true);
    
    HRESULT hr = E_FAIL;
    
    XalPlatformArgs xalPlatformArgs = {};
    xalPlatformArgs.redirectUri = REDIR_URI;
    
    XalInitArgs xalInitArgs = {};
    xalInitArgs.clientId     = CLIENT_ID;
    xalInitArgs.titleId      = TITLE_ID;
    xalInitArgs.sandbox      = SANDBOX;
    xalInitArgs.platformArgs = &xalPlatformArgs;
    
    hr = XalInitialize(&xalInitArgs, nullptr);
    if (FAILED(hr)) {
        SampleLog(LL_ERROR, "Failed to Init XAL!");
        return false;
    }
    SampleLog(LL_INFO, "XAL Init successful!");
    
    XblInitArgs xblInitArgs = { };
    xblInitArgs.scid = SC_ID;
    
    hr = XblInitialize(&xblInitArgs);
    if (FAILED(hr)) {
        SampleLog(LL_ERROR, "Failed to Init XboxLive!");
        return false;
    }
    SampleLog(LL_INFO, "XboxLive Init successful!");
    
    return true;
}

void GameScene::setXblContext(XblContextHandle xblContext)
{
    std::lock_guard<std::mutex> lock(m_xblContextMutex);
    m_xblContext = xblContext;
}

XblContextHandle GameScene::getXblContext()
{
    std::lock_guard<std::mutex> lock(m_xblContextMutex);
    return m_xblContext;
}

bool GameScene::hasXblContext()
{
    return m_xblContext != nullptr;
}

XalUserHandle GameScene::getCurrentUser()
{
    XalUserHandle user = nullptr;

    if (m_xblContext == nullptr) {
        SampleLog(LL_WARNING, "No XBL Context available, so no user available.");
        return nullptr;
    }

    HRESULT hr = XblContextGetUser(m_xblContext, &user);
    
    if (FAILED(hr))
    {
        SampleLog(LL_ERROR, "XblContextGetUser Failed!");
        SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(hr).c_str());
    }
    
    return user;
}

uint64_t GameScene::getCurrentUserId()
{
    XalUserHandle user = this->getCurrentUser();
    
    uint64_t xuid = 0;
    HRESULT hr = XalUserGetId(user, &xuid);
    
    if (FAILED(hr))
    {
        SampleLog(LL_ERROR, "XalUserGetId Failed!");
        SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(hr).c_str());
    }
    
    return xuid;
}
