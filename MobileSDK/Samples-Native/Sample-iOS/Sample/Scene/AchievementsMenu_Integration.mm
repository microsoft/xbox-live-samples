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
    if (this->achievementsMenuInstance)
    {
        AchievementsMenuView* achievementsView = (__bridge AchievementsMenuView*)this->achievementsMenuInstance;
        [achievementsView setHasNextResultsPage:value];
    }
}

bool AchievementsMenu_Integration::getHasNextResultsPage()
{
    if (this->achievementsMenuInstance)
    {
        AchievementsMenuView* achievementsView = (__bridge AchievementsMenuView*)this->achievementsMenuInstance;
        return [achievementsView getHasNextResultsPage];
    }
    return nullptr;
}

void AchievementsMenu_Integration::setAchievementsResultHandle(XblAchievementsResultHandle resultHandle)
{
    if (this->achievementsMenuInstance)
    {
        AchievementsMenuView* achievementsView = (__bridge AchievementsMenuView*)this->achievementsMenuInstance;
        [achievementsView setAchievementsResultHandle:resultHandle];
    }
}

XblAchievementsResultHandle AchievementsMenu_Integration::getAchievementsResultHandle()
{
    if (this->achievementsMenuInstance)
    {
        AchievementsMenuView* achievementsView = (__bridge AchievementsMenuView*)this->achievementsMenuInstance;
        return [achievementsView getAchievementsResultHandle];
    }
    return nullptr;
}
