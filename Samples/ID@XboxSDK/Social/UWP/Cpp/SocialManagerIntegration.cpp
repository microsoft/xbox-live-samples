// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
#include "pch.h"
#include <time.h>
#include "Social.h"

using namespace Concurrency;
using namespace Windows::Foundation::Collections;
using namespace Windows::Foundation;
using namespace xbox::services;
using namespace xbox::services::social::manager;

void Sample::InitializeSocialManager()
{
    m_socialManager = social_manager::get_singleton_instance();
}

void Sample::InitializeSocialManager(std::shared_ptr<xbox::services::system::xbox_live_user> user)
{
    m_socialManager = social_manager::get_singleton_instance();

    AddUserToSocialManager(user);
}

void
Sample::AddUserToSocialManager(
    _In_ std::shared_ptr<xbox::services::system::xbox_live_user> user
    )
{
    // Add the local user
    {
        std::lock_guard<std::mutex> guard(m_socialManagerLock);

        stringstream_t source;
        source << _T("Adding user ");
        source << user->gamertag();
        source << _T(" to SocialManager");
        m_console->WriteLine(source.str().c_str());

        m_socialManager->add_local_user(user, social_manager_extra_detail_level::preferred_color_level | social_manager_extra_detail_level::title_history_level);
        m_socialManager->set_rich_presence_polling_status(user, true);
    }

    // Setup the social groups
    CreateSocialGroupFromFilters(user, presence_filter::all, relationship_filter::friends);
    CreateSocialGroupFromFilters(user, presence_filter::all_online, relationship_filter::friends);
    CreateSocialGroupFromFilters(user, presence_filter::title_online, relationship_filter::friends);
    CreateSocialGroupFromFilters(user, presence_filter::all, relationship_filter::favorite);
}

void
Sample::RemoveUserFromSocialManager(
    _In_ std::shared_ptr<xbox::services::system::xbox_live_user> user
    )
{
    std::lock_guard<std::mutex> guard(m_socialManagerLock);

    stringstream_t source;
    source << _T("Removing user ");
    source << user->gamertag();
    source << _T(" from SocialManager");
    m_console->WriteLine(source.str().c_str());

    auto it = m_socialGroups.begin();
    while (it != m_socialGroups.end()) 
    {
        std::shared_ptr<xbox_social_user_group> group = *it;
        if (wcscmp(group->local_user()->xbox_user_id().c_str(), user->xbox_user_id().c_str()))
        {
            it = m_socialGroups.erase(it);
        }
        else 
        {
            ++it;
        }
    }

    m_socialManager->remove_local_user(user);
}

void
Sample::CreateSocialGroupFromList(
    _In_ std::shared_ptr<xbox::services::system::xbox_live_user> user,
    _In_ std::vector<string_t> xuidList
    )
{
    // Create a social group using a title-managed xuid list

    if(!xuidList.empty())
    {
        auto result = m_socialManager->create_social_user_group_from_list(user, xuidList);
        if (!result.err())
        {
            std::lock_guard<std::mutex> guard(m_socialManagerLock);
            m_socialGroups.push_back(result.payload());
        }
        else
        {
            stringstream_t source;
            source << _T("  Error ");
            source << result.err();
            source << _T(".");
            m_console->WriteLine(source.str().c_str());
        }
    }
}

void Sample::CreateSocialGroupFromFilters(
    _In_ std::shared_ptr<xbox::services::system::xbox_live_user> user,
    _In_ presence_filter presenceFilter,
    _In_ relationship_filter relationshipFilter
    )
{
    m_console->WriteLine(L"Creating Social Group.");

    auto result = m_socialManager->create_social_user_group_from_filters(user, presenceFilter, relationshipFilter);
    if (!result.err())
    {
        std::lock_guard<std::mutex> guard(m_socialManagerLock);
        m_socialGroups.push_back(result.payload());
    }
    else
    {
        stringstream_t source;
        source << _T("  Error ");
        source << result.err();
        source << _T(".");
        m_console->WriteLine(source.str().c_str());
    }
}

void Sample::DestorySocialGroupFromList(
    _In_ std::shared_ptr<xbox::services::system::xbox_live_user> user
    )
{
    std::lock_guard<std::mutex> guard(m_socialManagerLock);

    auto it = m_socialGroups.begin();
    while (it != m_socialGroups.end())
    {
        std::shared_ptr<xbox_social_user_group> group = *it;
        if (wcscmp(group->local_user()->xbox_user_id().c_str(), user->xbox_user_id().c_str()) == 0 &&
            group->social_user_group_type() == social_user_group_type::user_list_type)
        {
            m_socialManager->destroy_social_user_group(group);
            it = m_socialGroups.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Sample::DestroySocialGroup(
    _In_ std::shared_ptr<xbox::services::system::xbox_live_user> user,
    _In_ presence_filter presenceFilter,
    _In_ relationship_filter relationshipFilter
    )
{
    std::lock_guard<std::mutex> guard(m_socialManagerLock);

    auto it = m_socialGroups.begin();
    while (it != m_socialGroups.end())
    {
        std::shared_ptr<xbox_social_user_group> group = *it;
        if (wcscmp(group->local_user()->xbox_user_id().c_str(), user->xbox_user_id().c_str()) == 0 &&
            group->presence_filter_of_group() == presenceFilter && 
            group->relationship_filter_of_group() == relationshipFilter)
        {
            m_socialManager->destroy_social_user_group(group);
            it = m_socialGroups.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Sample::UpdateSocialManager()
{
    // Process events from the social manager
    // This should be called each frame update

#if PERF_COUNTERS
    auto perfInstance = performance_counters::get_singleton_instance();
    perfInstance->begin_capture(L"updates");
#endif
    auto socialEvents = m_socialManager->do_work();
    std::wstring text;

    for (const auto& evt : socialEvents)
    {
        switch (evt.event_type())
        {
            case social_event_type::local_user_added: text = L"local_user_added"; break;
            case social_event_type::local_user_removed: text = L"local_user_removed"; break;
            case social_event_type::profiles_changed: text = L"profiles_changed"; break;
            case social_event_type::social_relationships_changed: text = L"social_relationships_changed"; break;
            case social_event_type::social_user_group_loaded:
                text = L"social_user_group_loaded";
                RefreshUserList();
                break;
            case social_event_type::social_user_group_updated:
                text = L"social_user_group_updated";
                RefreshUserList();
                break;
            case social_event_type::presence_changed: 
                text = L"presence_changed"; 
                RefreshUserList();
                break;
            case social_event_type::users_added_to_social_graph: 
                text = L"users_added_to_social_graph"; 
                RefreshUserList();
                break;
            case social_event_type::users_removed_from_social_graph: 
                text = L"users_removed_from_social_graph"; 
                RefreshUserList();
            case social_event_type::unknown: text = L"unknown"; break;
                break;
        }

        stringstream_t source;
        source << _T("SocialManager event: ");
        source << text;
        source << _T(".");
        m_console->WriteLine(source.str().c_str());
    }
#if PERF_COUNTERS
    perfInstance->end_capture(L"updates");
#endif
}

std::vector<std::shared_ptr<xbox_social_user_group>> Sample::GetSocialGroups()
{
    std::lock_guard<std::mutex> guard(m_socialManagerLock);
    return m_socialGroups;
}



