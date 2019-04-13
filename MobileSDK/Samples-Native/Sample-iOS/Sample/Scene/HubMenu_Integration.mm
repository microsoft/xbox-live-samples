// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import "HubMenu_Integration.h"
#import <HubMenuView.h>

static HubMenu_Integration* s_HubInstance = nullptr;

HubMenu_Integration* HubMenu_Integration::getInstance()
{
    if (!s_HubInstance)
    {
        s_HubInstance = new (std::nothrow) HubMenu_Integration();
        s_HubInstance->init();
    }
    
    return s_HubInstance;
}

void HubMenu_Integration::init()
{
    SampleLog(LL_TRACE, "Initializing Hub Integration");
}

void HubMenu_Integration::setHubMenuHidden(bool hidden)
{
    if (this->hubMenuInstance)
    {
        HubMenuView* hubView = (__bridge HubMenuView*)this->hubMenuInstance;
        [hubView updateMenuHidden:hidden];
    }
}
