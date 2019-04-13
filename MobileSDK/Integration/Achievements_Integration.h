// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

HRESULT Achievements_GetAchievementsForTitle(
    _In_ XTaskQueueHandle asyncQueue,
    _In_ XblContextHandle xblContext,
    _In_ uint32_t skipItems,
    _In_ uint32_t maxItems);

HRESULT Achievements_GetNextResultsPage(
    _In_ XTaskQueueHandle asyncQueue,
    _In_ XblContextHandle xblContext,
    _In_ XblAchievementsResultHandle achievementsResultHandle,
    _In_ uint32_t maxItems);

HRESULT Achievements_GetAchievement(
    _In_ XTaskQueueHandle asyncQueue,
    _In_ XblContextHandle xblContext,
    _In_z_ const std::string& achievementId);

HRESULT Achievements_UpdateAchievement(
    _In_ XTaskQueueHandle asyncQueue,
    _In_ XblContextHandle xblContext,
    _In_z_ const std::string& achievementId,
    _In_ uint32_t percentComplete);

void Achievements_Gameplay_GetAchievementsForTitle(
    _In_ HRESULT result,
    _In_ XblAchievementsResultHandle achievementsResultHandle,
    _In_ XblAchievement* achievements,
    _In_ uint32_t achievementsCount,
    _In_ bool hasNextPage);

void Achievements_Gameplay_GetNextResultsPage(
    _In_ HRESULT result,
    _In_ XblAchievementsResultHandle achievementsResultHandle,
    _In_ XblAchievement* achievements,
    _In_ uint32_t achievementsCount,
    _In_ bool hasNextPage);

void Achievements_Gameplay_GetAchievement(
    _In_ HRESULT result,
    _In_ XblAchievement* achievements,
    _In_ uint32_t achievementsCount);

void Achievements_Gameplay_UpdateAchievement(
    _In_ HRESULT result);