// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

class HubMenu_Integration
{
public:
    static HubMenu_Integration* getInstance();
    
    void init();
    
    void setHubMenuHidden(bool hidden);
};
