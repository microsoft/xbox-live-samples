// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#define ID_NONE         0
#define ID_SIGNED_IN    1
#define ID_SIGNED_OUT   2

class AchievementsMenu_Integration
{
public:
    void* achievementsMenuInstance;
    
    static AchievementsMenu_Integration* getInstance();
    
    void init();
    
    void setHasNextResultsPage(bool value);
    bool getHasNextResultsPage();
    void setAchievementsResultHandle(XblAchievementsResultHandle resultHandle);
    XblAchievementsResultHandle getAchievementsResultHandle();
};
