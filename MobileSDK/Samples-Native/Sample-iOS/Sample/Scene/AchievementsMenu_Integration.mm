// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import "AchievementsMenu_Integration.h"
#import <AchievementsMenuView.h>

static AchievementsMenu_Integration* s_achievementsInstance = nullptr;

AchievementsMenu_Integration* AchievementsMenu_Integration::getInstance()
{
    if (!s_achievementsInstance)
    {
        s_achievementsInstance = new (std::nothrow) AchievementsMenu_Integration();
        s_achievementsInstance->init();
    }
    
    return s_achievementsInstance;
}

void AchievementsMenu_Integration::init()
{
    SampleLog(LL_TRACE, "Initializing Achievements Integration");
}

void AchievementsMenu_Integration::setHasNextResultsPage(bool value)
{
    [[AchievementsMenuView shared] setHasNextResultsPage:value];
}

bool AchievementsMenu_Integration::getHasNextResultsPage()
{
    return [[AchievementsMenuView shared] getHasNextResultsPage];
}

void AchievementsMenu_Integration::setAchievementsResultHandle(XblAchievementsResultHandle resultHandle)
{
    [[AchievementsMenuView shared] setAchievementsResultHandle:resultHandle];
}

XblAchievementsResultHandle AchievementsMenu_Integration::getAchievementsResultHandle()
{
    return [[AchievementsMenuView shared] getAchievementsResultHandle];
}
