// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#import <XSAPI_Integration.h>
#import <Game_Integration.h>
#import <AchievementsMenu_Integration.h>

#pragma region Achievements Gameplay Internal

std::string Achievements_Gameplay_ProgressToString(
    _In_ XblAchievement achievement)
{
    std::stringstream stream;

    if (achievement.progressState == XblAchievementProgressState::Achieved)
    {
        char buffer[32];
        std::tm* ptm = std::localtime(&achievement.progression.timeUnlocked);
        std::strftime(buffer, 32, "%Y-%m-%d %H:%M:%S", ptm);
        stream << "Achieved (" << buffer << ")";
    }
    else if (achievement.progressState == XblAchievementProgressState::InProgress)
    {
        stream << "Started "
               << "(" << achievement.progression.requirements[0].currentProgressValue
               << "/" << achievement.progression.requirements[0].targetProgressValue << ")";
    }
    else if (achievement.progressState == XblAchievementProgressState::NotStarted)
    {
        stream << "Not Started";
    }
    else if (achievement.progressState == XblAchievementProgressState::Unknown)
    {
        stream << "Unknown";
    }

    return stream.str();
}

void Achievements_Gameplay_DisplayAchievements(
    _In_ const XblAchievement* achievements,
    _In_ size_t achievementsCount)
{
    for (uint32_t i = 0; i < achievementsCount; ++i)
    {
        SampleLog(LL_INFO, "%s) %s", achievements[i].id, achievements[i].name);
        SampleLog(LL_INFO, "State: %s", Achievements_Gameplay_ProgressToString(achievements[i]).c_str());
    }
}

#pragma endregion

#pragma region Achievements Gameplay

void Achievements_Gameplay_GetAchievementsForTitle(
    _In_ HRESULT result,
    _In_ XblAchievementsResultHandle achievementsResultHandle,
    _In_ const XblAchievement* achievements,
    _In_ size_t achievementsCount,
    _In_ bool hasNextPage)
{
    // TODO: The game dev should implement logic below as desired to hook it up with the rest of the game

    if (SUCCEEDED(result))
    {
        Achievements_Gameplay_DisplayAchievements(achievements, achievementsCount);

        // Pass hasNextPage to Get Next Page UI button to set enabled or not
        AchievementsMenu_Integration::getInstance()->setHasNextResultsPage(hasNextPage);
        if (hasNextPage)
        {
            // Store Achievement Result Handle
            AchievementsMenu_Integration::getInstance()->setAchievementsResultHandle(achievementsResultHandle);
        }
        else
        {
            AchievementsMenu_Integration::getInstance()->setAchievementsResultHandle(nullptr);
        }
    }
}

void Achievements_Gameplay_GetNextResultsPage(
    _In_ HRESULT result,
    _In_ XblAchievementsResultHandle achievementsResultHandle,
    _In_ const XblAchievement* achievements,
    _In_ size_t achievementsCount,
    _In_ bool hasNextPage)
{
    // TODO: The game dev should implement logic below as desired to hook it up with the rest of the game

    if (SUCCEEDED(result))
    {
        Achievements_Gameplay_DisplayAchievements(achievements, achievementsCount);

        // Pass hasNextPage to Get Next Page UI button to set enabled or not
        AchievementsMenu_Integration::getInstance()->setHasNextResultsPage(hasNextPage);

        if (hasNextPage)
        {
            // Store Achievement Result Handle
            AchievementsMenu_Integration::getInstance()->setAchievementsResultHandle(achievementsResultHandle);
        }
        else
        {
            AchievementsMenu_Integration::getInstance()->setAchievementsResultHandle(nullptr);
        }
    }
}

void Achievements_Gameplay_GetAchievement(
    _In_ HRESULT result,
    _In_ const XblAchievement* achievements,
    _In_ size_t achievementsCount)
{
    // TODO: The game dev should implement logic below as desired to hook it up with the rest of the game

    if (SUCCEEDED(result))
    {
        Achievements_Gameplay_DisplayAchievements(achievements, achievementsCount);
    }
}

void Achievements_Gameplay_UpdateAchievement(
    _In_ HRESULT result)
{
    // TODO: The game dev should implement logic below as desired to hook it up with the rest of the game

    if (SUCCEEDED(result))
    {

    }
}

#pragma endregion
