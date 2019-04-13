// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "pch.h"
#include "GameLogic\Matchmaking.h"
#include "Utils\PerformanceCounters.h"
#include <time.h>

using namespace Concurrency;
using namespace Windows::Foundation::Collections;
using namespace Windows::Foundation;
using namespace xbox::services;
using namespace xbox::services::multiplayer;
using namespace xbox::services::multiplayer::manager;

void
Sample::InitializeMultiplayerManager(_In_ const string_t& templateName)
{
    m_multiplayerManager = multiplayer_manager::get_singleton_instance();
    m_multiplayerManager->initialize(templateName);
}

void Sample::DoWork()
{
    Concurrency::critical_section::scoped_lock lock(m_stateLock);
    if (m_multiplayerManager != nullptr)
    {
#if PERF_COUNTERS
        auto perfInstance = performance_counters::get_singleton_instance();
        perfInstance->begin_capture(L"no_updates");
        perfInstance->begin_capture(L"updates");
        m_multiplayerEventQueue = m_multiplayerManager->do_work();
        if (!m_multiplayerManager->_Is_dirty())
        {
            perfInstance->end_capture(L"no_updates");
        }
        else
        {
            perfInstance->end_capture(L"updates");
        }
#else
        m_multiplayerEventQueue = m_multiplayerManager->do_work();
#endif
        for (auto& multiplayerEvent : m_multiplayerEventQueue)
        {
            switch (multiplayerEvent.event_type())
            {
                case multiplayer_event_type::user_added:
                {
                    auto userAddedArgs = std::dynamic_pointer_cast<user_added_event_args>(multiplayerEvent.event_args());
                    if (!multiplayerEvent.err())
                    {
                        m_appState = APP_IN_GAME;
                        ChangeAppStates();
                    }
                    break;
                }

                case multiplayer_event_type::leave_game_completed:
                {
                    m_isLeavingGame = false;
                    m_findMatchBtn->SetImageId(3203);
                    break;
                }

                case multiplayer_event_type::find_match_completed:
                {
                    m_findMatchBtn->SetImageId(3204);
                    if (multiplayerEvent.err())
                    {
                        if (m_multiplayerManager->lobby_session()->local_members().empty())
                        {
                            LogErrorFormat(L"JoinGame failed: %S\n", multiplayerEvent.err_message().c_str());
                            m_appState = APP_MAIN_MENU;
                            ChangeAppStates();
                        }
                        else
                        {
                            m_findMatchBtn->SetImageId(3203);
                        }
                    }
                    break;
                }
            }
        }
    }
}

void Sample::AddLocalUser()
{
    assert(m_multiplayerManager != nullptr);
    if (m_liveResources->GetLiveContext() != nullptr)
    {
        Concurrency::critical_section::scoped_lock lock(m_stateLock);

        auto result = m_multiplayerManager->lobby_session()->add_local_user(m_liveResources->GetLiveContext()->user());
        if (result.err())
        {
            LogErrorFormat(L"AddLocalUser failed: %S\n", result.err_message().c_str());
            m_appState = APP_MAIN_MENU;
            return;
        }

        m_multiplayerManager->lobby_session()->set_local_member_properties(
            m_liveResources->GetLiveContext()->user(),
            L"Health", web::json::value::number(rand() % 100), 
            (void*) InterlockedIncrement(&m_multiplayerContext)
        );

        string_t connectionAddress = L"1.1.1.1";
#if QOS_ENABLED
        connectionAddress = Windows::Networking::XboxLive::XboxLiveDeviceAddress::GetLocal()->GetSnapshotAsBase64()->ToString()->Data()
#endif
        m_multiplayerManager->lobby_session()->set_local_member_connection_address(m_liveResources->GetLiveContext()->user(), connectionAddress, (void*) InterlockedIncrement(&m_multiplayerContext));
    }
}

void Sample::RemoveLocalUser()
{
    assert(m_multiplayerManager != nullptr);
    Concurrency::critical_section::scoped_lock lock(m_stateLock);

    m_multiplayerManager->lobby_session()->remove_local_user(m_liveResources->GetLiveContext()->user());
}

void Sample::UpdateJoinability()
{
    assert(m_multiplayerManager != nullptr);

    auto joinabilityVal = GetRandJoinabilityValue();
    auto commitResult = m_multiplayerManager->set_joinability(joinabilityVal);
    if (commitResult.err())
    {
        LogErrorFormat(L"Updating joinability failed: %S\n", commitResult.err_message().c_str());
    }
}

void Sample::InviteFriends()
{
    assert(m_multiplayerManager != nullptr);

    // Pass empty string if you don’t want a custom string added to the invite.
    auto result = m_multiplayerManager->lobby_session()->invite_friends(m_liveResources->GetLiveContext()->user(), L"", L"Join my game!!");
    if (result.err())
    {
        LogErrorFormat(L"InviteFriends failed: %S\n", result.err_message().c_str());
    }
}

void Sample::UpdateLocalMemberProperties()
{
    assert(m_multiplayerManager != nullptr);
    if (m_liveResources->GetLiveContext() != nullptr)
    {
        Concurrency::critical_section::scoped_lock lock(m_stateLock);

        m_multiplayerManager->lobby_session()->set_local_member_properties(
            m_liveResources->GetLiveContext()->user(),
            L"Health",
            web::json::value::number(rand() % 100),
            (void*) InterlockedIncrement(&m_multiplayerContext)
            );

        m_multiplayerManager->lobby_session()->set_local_member_properties(
            m_liveResources->GetLiveContext()->user(),
            L"Skill",
            web::json::value::number(rand() % 100),
            (void*) InterlockedIncrement(&m_multiplayerContext)
            );
    }
}

void Sample::UpdateLobbyProperties()
{
    assert(m_multiplayerManager != nullptr);
    if (m_multiplayerManager->lobby_session() != nullptr)
    {
        Concurrency::critical_section::scoped_lock lock(m_stateLock);

        xbox_live_result<void> commitResult;
        commitResult = m_multiplayerManager->lobby_session()->set_synchronized_properties(
            L"GameMode", web::json::value::string(g_gameModeStrings[GetRandomizedGameModeIndex()]), (void*)InterlockedIncrement(&m_multiplayerContext)
            );

        commitResult = m_multiplayerManager->lobby_session()->set_synchronized_properties(
            L"Map", web::json::value::string(g_mapStrings[GetRandomizedMapIndex()]), (void*)InterlockedIncrement(&m_multiplayerContext)
            );

        if (commitResult.err())
        {
            LogErrorFormat(L"UpdateLobbyProperties failed: %S\n", commitResult.err_message().c_str());
        }
    }
}

void Sample::SetLobbyHost()
{
    assert(m_multiplayerManager != nullptr);
    auto lobby = m_multiplayerManager->lobby_session();
    if (lobby != nullptr)
    {
        Concurrency::critical_section::scoped_lock lock(m_stateLock);

        // Set the first local member to be the host.
        xbox_live_result<void> commitResult = lobby->set_synchronized_host(lobby->local_members().at(0), (void*)InterlockedIncrement(&m_multiplayerContext));
        if (commitResult.err())
        {
            LogErrorFormat(L"SetLobbyHost failed: %S\n", commitResult.err_message().c_str());
        }
    }
}

void Sample::FindMatch()
{
    assert(m_multiplayerManager != nullptr);
    xbox_live_result<void> commitResult = m_multiplayerManager->find_match(PLAYER_SKILL_NO_QOS_HOPPER);
    if (commitResult.err())
    {
        LogErrorFormat(L"FindMatch failed: %S\n", commitResult.err_message().c_str());
    }
}

void Sample::CancelMatch()
{
    assert(m_multiplayerManager != nullptr);
    m_multiplayerManager->cancel_match();
}

void Sample::LeaveGameSession()
{
    assert(m_multiplayerManager != nullptr);
    xbox_live_result<void> result = m_multiplayerManager->leave_game();
    if (result.err())
    {
        m_isLeavingGame = false;
        LogErrorFormat(L"LeaveGameSession failed: %S\n", result.err_message().c_str());
    }
}

void Sample::UpdateGameProperties()
{
    assert(m_multiplayerManager != nullptr);
    if (m_multiplayerManager->game_session() != nullptr)
    {
        Concurrency::critical_section::scoped_lock lock(m_stateLock);
        xbox_live_result<void> commitResult;

        commitResult = m_multiplayerManager->game_session()->set_synchronized_properties(
            L"GameMode", web::json::value::string(g_gameModeStrings[GetRandomizedGameModeIndex()]), (void*)InterlockedIncrement(&m_multiplayerContext)
        );

        commitResult = m_multiplayerManager->game_session()->set_synchronized_properties(
            L"Map", web::json::value::string(g_mapStrings[GetRandomizedMapIndex()]), (void*)InterlockedIncrement(&m_multiplayerContext)
        );

        if (commitResult.err())
        {
            LogErrorFormat(L"UpdateGameProperties failed: %S\n", commitResult.err_message().c_str());
        }
    }
}

void Sample::SetGameHost()
{
    assert(m_multiplayerManager != nullptr);
    if (m_multiplayerManager->game_session() != nullptr)
    {
        Concurrency::critical_section::scoped_lock lock(m_stateLock);

        // Set the first local member to be the host.
        xbox_live_result<void> commitResult = m_multiplayerManager->game_session()->set_synchronized_host(m_multiplayerManager->lobby_session()->local_members().at(0), (void*)InterlockedIncrement(&m_multiplayerContext));
        if (commitResult.err())
        {
            LogErrorFormat(L"SetGameHost failed: %S\n", commitResult.err_message().c_str());
        }
    }
}

#if QOS_ENABLED
void
Sample::perform_qos_measurements()
{
    Windows::Networking::XboxLive::XboxLiveQualityOfServiceMeasurement^ qosMeasurement = ref new XboxLiveQualityOfServiceMeasurement();
    qosMeasurement->Metrics->Append(XboxLiveQualityOfServiceMetric::AverageInboundBitsPerSecond);
    qosMeasurement->Metrics->Append(XboxLiveQualityOfServiceMetric::AverageOutboundBitsPerSecond);
    qosMeasurement->Metrics->Append(XboxLiveQualityOfServiceMetric::AverageLatencyInMilliseconds);
    qosMeasurement->NumberOfProbesToAttempt = NUMBER_OF_PROBES;
    qosMeasurement->TimeoutInMilliseconds = TIMEOUT_IN_MILLISECONDS;

    for (auto addressDeviceToken : m_addressToDeviceTokenMap)
    {
        string_t sda = addressDeviceToken.first;
        Platform::String^ secureDeviceAddress = ref new Platform::String(sda.c_str());
        if (!secureDeviceAddress->IsEmpty())
        {
            qosMeasurement->DeviceAddresses->Append(XboxLiveDeviceAddress::CreateFromSnapshotBase64(secureDeviceAddress));
            m_addressToDeviceTokenMap[secureDeviceAddress->Data()] = addressDeviceToken.second;
        }
    }

    pplx::create_task(qosMeasurement->MeasureAsync())
    .then([this, qosMeasurement](pplx::task<void> t)
    {
        try
        {
            t.get();
        }
        catch (Platform::Exception^ ex)
        {
            LogCommentFormat(L"qos_measurements failed. No measurements found.\n");
            return;
        }

        std::map<string_t, std::map<XboxLiveQualityOfServiceMetric, uint64_t>> qosResultMap;
        for (auto measurement : qosMeasurement->MetricResults)
        {
            Platform::String^ address = measurement->DeviceAddress->GetSnapshotAsBase64();
            string_t deviceToken = m_addressToDeviceTokenMap.at(address->Data());

            auto status = measurement->Status;
            if (status == XboxLiveQualityOfServiceMeasurementStatus::InProgressWithProvisionalResults ||
                status == XboxLiveQualityOfServiceMeasurementStatus::Succeeded)
            {
                qosResultMap[deviceToken][measurement->Metric] = measurement->Value;
            }
        }

        auto measurments = std::make_shared<std::vector<multiplayer_quality_of_service_measurements>>();
        for (auto mapElement : qosResultMap)
        {
            string_t deviceToken = mapElement.first;
            if (!deviceToken.empty())
            {
                auto peermeasurement = mapElement.second;

                Windows::Foundation::TimeSpan latency;
                latency.Duration = peermeasurement[XboxLiveQualityOfServiceMetric::AverageLatencyInMilliseconds] * TICKS_PER_MILLISECOND;
                auto qosMeasurement = multiplayer_quality_of_service_measurements(
                    deviceToken,
                    std::chrono::duration_cast<std::chrono::milliseconds>(timeSpanTicks(latency.Duration)),
                    peermeasurement[XboxLiveQualityOfServiceMetric::AverageInboundBitsPerSecond],
                    peermeasurement[XboxLiveQualityOfServiceMetric::AverageOutboundBitsPerSecond],
                    L"{}"
                    );

                measurments->push_back(qosMeasurement);
            }
        }

        m_multiplayerManager->set_quality_of_service_measurements(measurments);
    });
}
#endif