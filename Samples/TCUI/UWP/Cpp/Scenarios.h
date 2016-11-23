//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
#pragma once

#include "MainPage.g.h"
#include "xsapi\services.h"

namespace Sample
{
    class Scenarios
    {
    public:
        void Scenario_GetUserProfileAsync(_In_ MainPage^ mainPage, _In_ std::shared_ptr<xbox::services::xbox_live_context> xboxLiveContext);

        void Scenario_PeoplePickerAsyncTask(_In_ MainPage^ mainPage, _In_ std::shared_ptr<xbox::services::xbox_live_context> xboxLiveContext);
        void Scenario_SendInviteAsyncTask(_In_ MainPage^ ui, _In_ std::shared_ptr<xbox::services::xbox_live_context> xboxLiveContext);
        void Scenario_ShowProfileCardUI(_In_ MainPage^ ui, _In_ std::shared_ptr<xbox::services::xbox_live_context> xboxLiveContext);
        void Scenario_ShowChangeFriendRelationshipUI(_In_ MainPage^ ui, _In_ std::shared_ptr<xbox::services::xbox_live_context> xboxLiveContext);
        void Scenario_ShowTitleAchievementsUI(_In_ MainPage^ ui, _In_ std::shared_ptr<xbox::services::xbox_live_context> xboxLiveContext);
        void CreateAndJoinMultiplayerSession(_In_ MainPage^ ui, _In_ std::shared_ptr<xbox::services::xbox_live_context> xboxLiveContext);
        void Scenario_CheckGamingPrivilegeWithUI(_In_ MainPage^ ui, _In_ std::shared_ptr<xbox::services::xbox_live_context> xboxLiveContext);

    private:
        std::shared_ptr<xbox::services::multiplayer::multiplayer_session> m_session;
    };
}
