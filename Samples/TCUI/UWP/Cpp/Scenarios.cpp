//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
#include "pch.h"
#include "Scenarios.h"
#include "MainPage.xaml.h"
#include <ppltasks.h>
#include <mutex>

using namespace Sample;
using namespace concurrency;
using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Security::Cryptography;
using namespace Windows::Security::Authentication::OnlineId;

using namespace xbox::services;
using namespace xbox::services::social;
using namespace xbox::services::system;
using namespace xbox::services::presence;

std::mutex g_blockOfTextLock;

#define THROW_CPP_RUNTIME_IF(x,y) if ( x ) { throw std::runtime_error(y); }
string_t create_guid(_In_ bool removeBraces);

void Scenarios::Scenario_GetUserProfileAsync(_In_ MainPage^ ui, _In_ std::shared_ptr<xbox_live_context> xboxLiveContext)
{
    ui->Log(L"Calling get_user_profile...");

    xboxLiveContext->settings()->set_http_retry_delay(std::chrono::seconds(10));
    xboxLiveContext->settings()->set_enable_service_call_routed_events(true);

    xboxLiveContext->profile_service().get_user_profile(xboxLiveContext->user()->xbox_user_id())
    .then([ui](task<xbox::services::xbox_live_result<xbox::services::social::xbox_user_profile>> t) // use task_continuation_context::use_current() to make the continuation task running in current apartment 
    {
        xbox::services::xbox_live_result<xbox::services::social::xbox_user_profile> result = t.get();

        if (!result.err())
        {
            const auto& profile = result.payload();
            std::lock_guard<std::mutex> lockGuard(g_blockOfTextLock);
            ui->Log(L"");
            ui->Log(L"----------------");
            ui->Log(L"get_user_profile result:");
            ui->Log(StringFormat(L"app_display_name: %s", profile.app_display_name().c_str()));
            //ui->Log(StringFormat(L"app_display_picture_resize_uri: %s", profile.app_display_picture_resize_uri().to_string().c_str()));
            ui->Log(StringFormat(L"game_display_name: %s", profile.game_display_name().c_str()));
            //ui->Log(StringFormat(L"game_display_picture_resize_uri: %s", profile.game_display_picture_resize_uri().to_string().c_str()));
            ui->Log(StringFormat(L"gamerscore: %s", profile.gamerscore().c_str()));
            ui->Log(StringFormat(L"gamertag: %s", profile.gamertag().c_str()));
            ui->Log(StringFormat(L"xbox_user_id: %s", profile.xbox_user_id().c_str()));
        }
        else
        {
            ui->Log(StringFormat(L"get_user_profile failed: %S %s", result.err().message().c_str(), result.err_message().c_str()));
        }
    }, task_continuation_context::use_current());
}

void Scenarios::Scenario_PeoplePickerAsyncTask(_In_ MainPage^ ui, _In_ std::shared_ptr<xbox_live_context> xboxLiveContext)
{
    ui->Log(L"Calling PeoplePickerAsyncTask...");
    std::vector<string_t> xboxUserIds = { L"2814659809402826", L"2814613569642996", L"2814632956486799" };
    std::vector<string_t> preselectedXboxUserIds = { L"2814613569642996" };

    xbox::services::system::title_callable_ui::show_player_picker_ui(
        L"Pick people",
        xboxUserIds,
        preselectedXboxUserIds,
        1,
        20
        )
    .then([ui](xbox_live_result<std::vector<string_t>> xblResult) 
    {
        if(!xblResult.err())
        {
            auto selectedUsers = xblResult.payload();
            std::lock_guard<std::mutex> lockGuard(g_blockOfTextLock);
            ui->Log(L"");
            ui->Log(L"----------------");
            ui->Log(StringFormat(L"show_player_picker_ui results: [%d people picked]", selectedUsers.size()));
            for(auto& selectedUser : selectedUsers)
            {
                ui->Log(ref new Platform::String(selectedUser.c_str()));
            }
        }
        else
        {
            std::lock_guard<std::mutex> lockGuard(g_blockOfTextLock);
            ui->Log(L"");
            ui->Log(L"----------------");
            ui->Log(StringFormat(L"show_player_picker_ui failed: %S %s", xblResult.err().message().c_str(), xblResult.err_message().c_str()));
        }
    });
}

void
Scenarios::CreateAndJoinMultiplayerSession(
    _In_ MainPage^ ui,
    _In_ std::shared_ptr<xbox_live_context> xboxLiveContext
    )
{
    string_t sessionId = create_guid(true);
    string_t sessionTemplateName = L"GameSession10PublicNoActive";
    xbox::services::multiplayer::multiplayer_session_reference sessionRef = xbox::services::multiplayer::multiplayer_session_reference(
        xboxLiveContext->application_config()->scid(),
        sessionTemplateName,
        sessionId
        );

    std::shared_ptr<xbox::services::multiplayer::multiplayer_session> session = std::make_shared<xbox::services::multiplayer::multiplayer_session>(
        xboxLiveContext->user()->xbox_user_id(),
        sessionRef
        );

    session->join();
    session->set_current_user_status(xbox::services::multiplayer::multiplayer_session_member_status::active);

    xboxLiveContext->multiplayer_service().write_session(session, xbox::services::multiplayer::multiplayer_session_write_mode::update_or_create_new)
    .then([ui](xbox_live_result<std::shared_ptr<xbox::services::multiplayer::multiplayer_session>> xblResult)
    {
        if (!xblResult.err())
        {
            {
                std::lock_guard<std::mutex> lockGuard(g_blockOfTextLock);
                ui->Log(L"");
                ui->Log(L"----------------");
                ui->Log("Multiplayer session created");
            }
            std::shared_ptr<xbox::services::multiplayer::multiplayer_session> newSession;

            newSession = xblResult.payload();
        }
        else
        {
            std::lock_guard<std::mutex> lockGuard(g_blockOfTextLock);
            ui->Log(L"");
            ui->Log(L"----------------");
            ui->Log(StringFormat(L"session creation failed: %S %S", xblResult.err().message().c_str(), xblResult.err_message().c_str()));
        }
    }).wait();

    m_session = session;
    xboxLiveContext->multiplayer_service().set_activity(m_session->session_reference()).wait();
}

void Scenarios::Scenario_SendInviteAsyncTask(_In_ MainPage^ ui, _In_ std::shared_ptr<xbox_live_context> xboxLiveContext)
{
    ui->Log(L"Calling show_game_invite_ui...");

    if (m_session == nullptr || m_session->session_reference().is_null())
    {
        ui->Log(L"Error session not created.");
        return;
    }

    xbox::services::system::title_callable_ui::show_game_invite_ui(
        m_session->session_reference(),
        L"Please join me"
        )
    .then([ui](xbox_live_result<void> xblResult) 
    {
        if(!xblResult.err())
        {
            std::lock_guard<std::mutex> lockGuard(g_blockOfTextLock);
            ui->Log(L"");
            ui->Log(L"----------------");
            ui->Log(L"show_game_invite_ui succeeded");
        }
        else
        {
            std::lock_guard<std::mutex> lockGuard(g_blockOfTextLock);
            ui->Log(L"");
            ui->Log(L"----------------");
            ui->Log(StringFormat(L"show_game_invite_ui failed: %S %s", xblResult.err().message().c_str(), xblResult.err_message().c_str()));
        }
    });
}

void Scenarios::Scenario_ShowProfileCardUI(_In_ MainPage^ ui, _In_ std::shared_ptr<xbox_live_context> xboxLiveContext)
{
    ui->Log(L"Calling show_profile_card_ui...");

    xbox::services::system::title_callable_ui::show_profile_card_ui(
        L"2814613569642996"
        )
    .then([ui](xbox_live_result<void> xblResult)
    {
        if (!xblResult.err())
        {
            std::lock_guard<std::mutex> lockGuard(g_blockOfTextLock);
            ui->Log(L"");
            ui->Log(L"----------------");
            ui->Log(L"show_profile_card_ui succeeded");
        }
        else
        {
            std::lock_guard<std::mutex> lockGuard(g_blockOfTextLock);
            ui->Log(L"");
            ui->Log(L"----------------");
            ui->Log(StringFormat(L"show_profile_card_ui failed: %S %s", xblResult.err().message().c_str(), xblResult.err_message().c_str()));
        }
    });
}

void Scenarios::Scenario_ShowChangeFriendRelationshipUI(_In_ MainPage^ ui, _In_ std::shared_ptr<xbox_live_context> xboxLiveContext)
{
    ui->Log(L"Calling show_change_friend_relationship_ui...");

    xbox::services::system::title_callable_ui::show_change_friend_relationship_ui(
        L"2814613569642996"
        )
    .then([ui](xbox_live_result<void> xblResult)
    {
        if (!xblResult.err())
        {
            std::lock_guard<std::mutex> lockGuard(g_blockOfTextLock);
            ui->Log(L"");
            ui->Log(L"----------------");
            ui->Log(L"show_change_friend_relationship_ui succeeded");
        }
        else
        {
            std::lock_guard<std::mutex> lockGuard(g_blockOfTextLock);
            ui->Log(L"");
            ui->Log(L"----------------");
            ui->Log(StringFormat(L"show_change_friend_relationship_ui failed: %S %s", xblResult.err().message().c_str(), xblResult.err_message().c_str()));
        }
    });
}

void Scenarios::Scenario_ShowTitleAchievementsUI(_In_ MainPage^ ui, _In_ std::shared_ptr<xbox_live_context> xboxLiveContext)
{
    ui->Log(L"Calling show_title_achievements_ui...");

    xbox::services::system::title_callable_ui::show_title_achievements_ui(
        xboxLiveContext->application_config()->title_id()
        )
    .then([ui](xbox_live_result<void> xblResult)
    {
        if (!xblResult.err())
        {
            std::lock_guard<std::mutex> lockGuard(g_blockOfTextLock);
            ui->Log(L"");
            ui->Log(L"----------------");
            ui->Log(L"show_title_achievements_ui succeeded");
        }
        else
        {
            std::lock_guard<std::mutex> lockGuard(g_blockOfTextLock);
            ui->Log(L"");
            ui->Log(L"----------------");
            ui->Log(StringFormat(L"show_title_achievements_ui failed: %S %s", xblResult.err().message().c_str(), xblResult.err_message().c_str()));
        }
    });
}

string_t create_guid(_In_ bool removeBraces)
{
#ifdef _WIN32
    GUID guid = { 0 };
    THROW_CPP_RUNTIME_IF(FAILED(CoCreateGuid(&guid)), "");

    WCHAR wszGuid[50];
    THROW_CPP_RUNTIME_IF(FAILED(::StringFromGUID2(
        guid,
        wszGuid,
        ARRAYSIZE(wszGuid)
        )), "");

    string_t strGuid = wszGuid;
#else
    uuid_t uuid;
    uuid_generate_random(uuid);
    char s[37] = { 0 };
    uuid_unparse(uuid, s);
    string_t strGuid = s;
#endif
    if (removeBraces)
    {
        if (strGuid.length() > 3 && strGuid[0] == L'{')
        {
            // Remove the { } 
            strGuid.erase(0, 1);
            strGuid.erase(strGuid.end() - 1, strGuid.end());
        }
    }

    return strGuid;
}


void Scenarios::Scenario_CheckGamingPrivilegeWithUI(_In_ MainPage^ ui, _In_ std::shared_ptr<xbox_live_context> xboxLiveContext)
{
    ui->Log(L"Calling check_gaming_privilege_with_ui with gaming_privilege::multiplayer_sessions");
    ui->Log(L"If you want to see the dialog, change your privacy settings");
    ui->Log(L"and block multiplayer sessions for this account using the Xbox app or console");

    string_t friendlyMessage; // Fill this out if you want to provde a custom message, otherwise leave blank.

    xbox::services::system::title_callable_ui::check_gaming_privilege_with_ui(
        gaming_privilege::multiplayer_sessions, 
        friendlyMessage
        )
    .then([ui](xbox_live_result<bool> xblResult)
    {
        if (!xblResult.err())
        {
            std::lock_guard<std::mutex> lockGuard(g_blockOfTextLock);
            ui->Log(L"");
            ui->Log(L"----------------");
            ui->Log(StringFormat(L"check_gaming_privilege_with_ui succeeded.  Result: %d", xblResult.payload()));
        }
        else
        {
            std::lock_guard<std::mutex> lockGuard(g_blockOfTextLock);
            ui->Log(L"");
            ui->Log(L"----------------");
            ui->Log(StringFormat(L"check_gaming_privilege_with_ui failed: %S %s", xblResult.err().message().c_str(), xblResult.err_message().c_str()));
        }
    });
}
