// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "Game_Integration.h"
#include <Identity_Integration.h>
#include <XSAPI_Integration.h>

static Game_Integration* s_gInt = nullptr;

Game_Integration* Game_Integration::getInstance()
{
    if(!s_gInt)
    {
        s_gInt = new (std::nothrow) Game_Integration();
    }

    return s_gInt;
}

bool Game_Integration::Init()
{
    return XsapiInit();
}

void Game_Integration::Cleanup()
{
    XblCleanup();
}

bool Game_Integration::XsapiInit()
{
    HRESULT hr = E_FAIL;

    std::string scid = "ad560100-4bff-49c2-a4d6-44c431374b88";
    std::string clientId = "000000004824156C";
    uint32_t titleId = 825707400ull;
    std::string sandbox = "XDKS.1";
    std::string redirUri = "ms-xal-" + clientId + "://auth";
    
    XalPlatformArgs xalPlatformArgs = {};
    xalPlatformArgs.redirectUri = redirUri.c_str();
    
    XalInitArgs xalInitArgs = {};
    xalInitArgs.clientId     = clientId.c_str();
    xalInitArgs.titleId      = titleId;
    xalInitArgs.sandbox      = sandbox.c_str();
    xalInitArgs.platformArgs = &xalPlatformArgs;
    
    hr = XalInitialize(&xalInitArgs, nullptr);
    ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to Init XAL!");
    SampleLog(LL_DEBUG, "XAL Init successful!");

    XblInitArgs xblInitArgs = { };
    xblInitArgs.scid = scid.c_str();

    hr = XblInitialize(&xblInitArgs);
    ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to Init XBoxLive!");
    SampleLog(LL_DEBUG, "Xbl Init successful!");

    return true;
}
