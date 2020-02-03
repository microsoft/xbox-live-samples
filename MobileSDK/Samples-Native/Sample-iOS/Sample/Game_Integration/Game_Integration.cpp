// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import "Game_Integration.h"
#import <XSAPI_Integration.h>
#import "IdentityMenu_Integration.h"

#define SC_ID       "ad560100-4bff-49c2-a4d6-44c431374b88"
#define CLIENT_ID   "000000004824156C"
#define REDIR_URI   "ms-xal-000000004824156C://auth"
#define TITLE_ID    825707400ull
#define SANDBOX     "XDKS.1"

static Game_Integration* s_gameScene = nullptr;

Game_Integration* Game_Integration::getInstance()
{
    if (!s_gameScene)
    {
        s_gameScene = new (std::nothrow) Game_Integration();
        s_gameScene->init();
    }
    
    return s_gameScene;
}

void Game_Integration::init()
{
    SampleLog(LL_TRACE, "Entering Game Scene");
}

bool Game_Integration::xboxLive_init()
{
    // Set Debugger Trace Level for XSAPI
    HCSettingsSetTraceLevel(HCTraceLevel::Verbose);
    HCTraceSetTraceToDebugger(true);
    
    HRESULT hr = E_FAIL;
    
    XalInitArgs xalInitArgs = {};
    xalInitArgs.clientId     = CLIENT_ID;
    xalInitArgs.titleId      = TITLE_ID;
    xalInitArgs.sandbox      = SANDBOX;
    xalInitArgs.redirectUri  = REDIR_URI;
    
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

void Game_Integration::setXblContext(XblContextHandle xblContext)
{
    std::lock_guard<std::mutex> lock(m_xblContextMutex);
    m_xblContext = xblContext;
}

void Game_Integration::setXalUser(XalUserHandle xalUser)
{
    std::lock_guard<std::mutex> lock(m_xblContextMutex);
    if(m_xalUser){
        XalUserCloseHandle(m_xalUser);
    }
    m_xalUser = xalUser;
}


XblContextHandle Game_Integration::getXblContext()
{
    std::lock_guard<std::mutex> lock(m_xblContextMutex);
    return m_xblContext;
}

bool Game_Integration::hasXblContext()
{
    return m_xblContext != nullptr;
}

XalUserHandle Game_Integration::getCurrentUser()
{
    return m_xalUser;
}

uint64_t Game_Integration::getCurrentUserId()
{
    
    uint64_t xuid = 0;
    HRESULT hr = XalUserGetId(m_xalUser, &xuid);
    
    if (FAILED(hr))
    {
        SampleLog(LL_ERROR, "XalUserGetId Failed!");
        SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(hr).c_str());
    }
    
    return xuid;
}
