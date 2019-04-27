//--------------------------------------------------------------------------------------
// InGameChatUWP.cpp
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "InGameChatUWP.h"

#include "ATGColors.h"

extern void ExitSample();

using namespace DirectX;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace xbox::services::game_chat_2;
using namespace Microsoft::Xbox::Samples::NetworkMesh;
using namespace xbox::services;
using namespace xbox::services::multiplayer;
using namespace xbox::services::multiplayer::manager;
using namespace InGameChat;

using Microsoft::WRL::ComPtr;

Sample* Sample::s_this = nullptr;

Sample::Sample() :
    m_inLobby(true),
    m_inChat(false),
    m_selectPlayer(false),
    m_searching(false),
    m_refreshTimer(1.0f),
    m_needNames(false),
    m_loadingNames(false),
    m_meshInitialized(false),
    m_leavingToJoin(false),
    m_networkReady(false)
{
    s_this = this;

    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);

    // Set up UI using default fonts and colors.
    ATG::UIConfig uiconfig;
    m_ui = std::make_shared<ATG::UIManager>(uiconfig);
}

// Initialize the Direct3D resources required to run.
void Sample::Initialize(IUnknown* window, int width, int height, DXGI_MODE_ROTATION rotation)
{
    m_gamePad = std::make_unique<GamePad>();

    m_keyboard = std::make_unique<Keyboard>();
    m_keyboard->SetWindow(reinterpret_cast<ABI::Windows::UI::Core::ICoreWindow*>(window));

    m_mouse = std::make_unique<Mouse>();
    m_mouse->SetWindow(reinterpret_cast<ABI::Windows::UI::Core::ICoreWindow*>(window));

    Windows::Networking::XboxLive::XboxLiveDeviceAddress::GetLocal()->SnapshotChanged += ref new Windows::Foundation::TypedEventHandler<Windows::Networking::XboxLive::XboxLiveDeviceAddress^,Platform::Object^>([this](Windows::Networking::XboxLive::XboxLiveDeviceAddress^, Platform::Object^)
    {
        m_networkReady = true;
    });

    SetupUI();

    m_deviceResources->SetWindow(window, width, height, rotation);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    m_multiplayerManager = multiplayer_manager::get_singleton_instance();
    m_multiplayerManager->initialize(GAME_MULTIPLAYER_SESSION_TEMPLATE_NAME);

    m_userList->LoadImages(m_deviceResources->GetD3DDevice());
    m_chatList->LoadImages(m_deviceResources->GetD3DDevice());
    m_playerList->LoadImages(m_deviceResources->GetD3DDevice());

    UpdateControllerUIState();

    m_ui->FindPanel<ATG::Overlay>(200)->Show();
}

#pragma region Frame Update
// Executes basic render loop.
void Sample::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

// Updates the world.
void Sample::Update(DX::StepTimer const& timer)
{
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update");

    float elapsedTime = float(timer.GetElapsedSeconds());

    if (m_refreshTimer <= 0.0f && m_needNames)
    {
        m_refreshTimer = 60.0f;
        LoadUserDisplayNames();
    }
    else
    {
        m_refreshTimer -= elapsedTime;
    }

    UpdateControllerUIState();

    if (m_activationArgs != nullptr)
    {
        if (m_inLobby)
        {
            JoinSession();
        }
        else if (!m_leavingToJoin)
        {
            m_leavingToJoin = true;

            if (m_inChat)
            {
                LeaveChat();
                GetChatIntegrationLayer()->ProcessDataFrames();
            }

            m_multiplayerManager->lobby_session()->remove_local_user(GetCurrentUser());
        }
    }

    auto leave = false;
    auto exiting = false;

    auto pad = m_gamePad->GetState(0);
    auto kbd = m_keyboard->GetState();

    if (pad.IsConnected())
    {
        m_gamePadButtons.Update(pad);
    }

    m_keyboardButtons.Update(kbd);

    if (m_inLobby)
    {
        if (m_gamePadButtons.y == GamePad::ButtonStateTracker::PRESSED || m_keyboardButtons.IsKeyPressed(DirectX::Keyboard::Keys::Y))
        {
            SignInUser();
        }
        else if (m_gamePadButtons.a == GamePad::ButtonStateTracker::PRESSED || m_keyboardButtons.IsKeyPressed(DirectX::Keyboard::Keys::A))
        {
            if (GetCurrentUser() != nullptr)
            {
                JoinSession();
            }
        }
        else if (m_gamePadButtons.b == GamePad::ButtonStateTracker::PRESSED || m_keyboardButtons.IsKeyPressed(DirectX::Keyboard::Keys::B))
        {
            ExitSample();
            exiting = true;
        }
        else if (m_gamePadButtons.x == GamePad::ButtonStateTracker::PRESSED || m_keyboardButtons.IsKeyPressed(DirectX::Keyboard::Keys::X))
        {
            if (GetCurrentUser() != nullptr)
            {
                SearchJoinableUsers();
            }
        }
    }
    else if (m_selectPlayer)
    {
        if (m_gamePadButtons.b == GamePad::ButtonStateTracker::PRESSED || m_keyboardButtons.IsKeyPressed(DirectX::Keyboard::Keys::B))
        {
            leave = true;
        }
        else if (m_gamePadButtons.x == GamePad::ButtonStateTracker::PRESSED || m_keyboardButtons.IsKeyPressed(DirectX::Keyboard::Keys::X))
        {
            SearchJoinableUsers();
        }
    }
    else
    {
        if (m_gamePadButtons.b == GamePad::ButtonStateTracker::PRESSED || m_keyboardButtons.IsKeyPressed(DirectX::Keyboard::Keys::B))
        {
            m_multiplayerManager->lobby_session()->remove_local_user(GetCurrentUser());

            if (m_inChat)
            {
                LeaveChat();
            }
        }
        else if (m_gamePadButtons.y == GamePad::ButtonStateTracker::PRESSED || m_keyboardButtons.IsKeyPressed(DirectX::Keyboard::Keys::Y))
        {
            SendTextMessage(L"This is a text message!");
        }
        else if (m_gamePadButtons.menu == GamePad::ButtonStateTracker::PRESSED || m_keyboardButtons.IsKeyPressed(DirectX::Keyboard::Keys::M))
        {
            InviteUsers();
        }
        else if (m_gamePadButtons.leftShoulder == GamePad::ButtonStateTracker::PRESSED || m_keyboardButtons.IsKeyPressed(DirectX::Keyboard::Keys::D))
        {
            ChangeChannel(-1);
        }
        else if (m_gamePadButtons.rightShoulder == GamePad::ButtonStateTracker::PRESSED || m_keyboardButtons.IsKeyPressed(DirectX::Keyboard::Keys::U))
        {
            ChangeChannel(1);
        }
    }

    m_gamePadButtons.Reset();

    pad = m_gamePad->GetState(0);
    if (pad.IsConnected())
    {
        std::lock_guard<std::mutex> lock(m_uiLock);

        m_gamePadButtons.Update(pad);

        m_ui->Update(elapsedTime, pad);
    }
    else
    {
        m_gamePadButtons.Reset();
    }
    
    m_ui->Update(elapsedTime, *m_mouse, *m_keyboard);

    if (leave)
    {
        m_selectPlayer = false;
        LeaveSession();
    }

    if (!exiting)
    {
        ProcessMpmMessages();
    }

    PIXEndEvent();
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Sample::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    auto context = m_deviceResources->GetD3DDeviceContext();
    PIXBeginEvent(context, PIX_COLOR_DEFAULT, L"Render");

    {
        std::lock_guard<std::mutex> lock(m_uiLock);
        m_ui->Render();
    }

    if (!m_inLobby && !m_selectPlayer)
    {
        m_console->Render();
    }

    PIXEndEvent(context);

    // Show the new frame.
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Present");
    m_deviceResources->Present();
    PIXEndEvent();
}

// Helper method to clear the back buffers.
void Sample::Clear()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    PIXBeginEvent(context, PIX_COLOR_DEFAULT, L"Clear");

    // Clear the views.
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, ATG::Colors::Background);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    PIXEndEvent(context);
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Sample::OnActivated()
{
}

void Sample::OnDeactivated()
{
}

void Sample::OnSuspending()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    context->ClearState();

    m_deviceResources->Trim();

    LeaveSession();
}

void Sample::OnResuming()
{
    m_timer.ResetElapsedTime();
    m_gamePadButtons.Reset();
    m_keyboardButtons.Reset();

    // Reset UI on return from suspend.
    m_ui->Reset();
}

void Sample::OnWindowSizeChanged(int width, int height, DXGI_MODE_ROTATION rotation)
{
    if (!m_deviceResources->WindowSizeChanged(width, height, rotation))
    {
        return;
    }

    CreateWindowSizeDependentResources();
}

void Sample::ValidateDevice()
{
    m_deviceResources->ValidateDevice();
}

void Sample::OnProtocolActivation(Windows::ApplicationModel::Activation::IProtocolActivatedEventArgs ^ args)
{
    m_activationArgs = args;
}

// Properties
void Sample::GetDefaultSize(int& width, int& height) const
{
    width = 1600;
    height = 900;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Sample::CreateDeviceDependentResources()
{
    auto context = m_deviceResources->GetD3DDeviceContext();

    m_console = std::make_unique <DX::TextConsole>(context, L"Assets\\Consolas_8.spritefont");
    m_console->SetDebugOutput(true);

    // Let UI create Direct3D resources.
    m_ui->RestoreDevice(context);
}

// Allocate all memory resources that change on a window SizeChanged event.
void Sample::CreateWindowSizeDependentResources()
{
    RECT fullscreen = m_deviceResources->GetOutputSize();

    // Let UI know the size of our rendering viewport.
    m_ui->SetWindow(fullscreen);

    m_console->SetWindow(*(m_ui->FindControl<ATG::Image>(300, 317)->GetRectangle()));
}

void Sample::OnDeviceLost()
{
    m_console->ReleaseDevice();
    m_ui->ReleaseDevice();
}

void Sample::OnDeviceRestored()
{
    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}
#pragma endregion


#pragma region GUI Setup and Drawing
void Sample::SetupUI()
{
    m_ui->LoadLayout(L".\\Assets\\Layout.csv", L".\\Assets");

    // Setup user list repeaters
    auto loc = POINT{ 169, 149 };
    auto pos = SIZE{ 610, 76 };

    // UserList is the 'local lobby' user list
    m_userList = std::make_unique<UserRepeater>(m_ui, loc, pos, 8000);
    m_userList->SetReadOnly(true);

    // ChatList is the active session user list
    m_chatList = std::make_unique<UserRepeater>(m_ui, loc, pos, 9000);
    m_chatList->SetSelectedCallback([this](unsigned index)
    {
        // Mute the selected user
        auto players = GetChatIntegrationLayer()->GetChatUsersXuids();

        if (players.size() > index)
        {
            GetChatIntegrationLayer()->ChangeChatUserMuteState(players.at(index).c_str());
        }
    });

    // PlayerList is for joining friends
    m_playerList = std::make_unique<UserRepeater>(m_ui, loc, pos, 7000);
    m_playerList->SetSelectedCallback([this](unsigned index)
    {
        // Attempt to join the selected user's session
        m_selectPlayer = false;

        m_joinXuid = m_joinableXuids[index];
        JoinSession();
    });

    // Populate each list with empty items to start
    auto users = std::vector<std::shared_ptr<UserListItem>>();

    for (auto x = 0; x < MAXUSERS; x++)
    {
        users.push_back(std::make_shared<UserListItem>());
    }

    m_userList->GenerateList(200, users, 2);
    m_chatList->GenerateList(300, users, 2);
    m_playerList->GenerateList(400, users, 2);
}

void Sample::UpdateControllerUIState()
{
    // Update the current user list items

    auto users = std::vector<std::shared_ptr<UserListItem>>();

    if (m_inLobby)
    {
        if (GetCurrentUser() == nullptr)
        {
            users.push_back(std::make_shared<UserListItem>(true));
        }
        else
        {
            users.push_back(std::make_shared<UserListItem>(GetCurrentUser()));
        }

        m_userList->UpdateList(users);
    }
    else if (m_selectPlayer)
    {
        // Populated via async call to Xbox Live
    }
    else
    {
        // Populate the list from the current GameChat session

        auto players = GetChatIntegrationLayer()->GetChatUsersXuids();

        for (unsigned i = 0; i < 8; i++)
        {
            if (i < players.size())
            {
                users.push_back(std::make_shared<UserListItem>(GetChatIntegrationLayer()->GetChatUserByXboxUserId(players.at(i).c_str())));
            }
            else
            {
                users.push_back(std::make_shared<UserListItem>());
            }
        }

        m_chatList->UpdateList(users);
    }
}
#pragma endregion

#pragma region UI Actions
void Sample::SignInUser()
{
    DebugTrace("Sample::SignInUser()\n");

    m_user = std::make_shared<system::xbox_live_user>();

    concurrency::create_task(
        m_user->signin_silently(nullptr)
    ).then([this](xbox_live_result<system::sign_in_result> t)
    {
        if (!t.err())
        {
            if (t.payload().status() == system::sign_in_status::user_interaction_required)
            {
                concurrency::create_task(
                    m_user->signin(nullptr)
                ).then([this](xbox_live_result<system::sign_in_result> t)
                {
                    if (!t.err())
                    {
                        auto result = t.payload();

                        if (result.status() == system::sign_in_status::success)
                        {
                            m_context = std::make_shared<xbox_live_context>(m_user);
                        }
                        else
                        {
                            m_user = nullptr;
                        }
                    }
                    {
                        m_user = nullptr;
                    }
                });
            }
            else
            {
                m_context = std::make_shared<xbox_live_context>(m_user);
            }
        }
        else
        {
            m_user = nullptr;
        }
    });
}

void Sample::InviteUsers()
{
    DebugTrace("Sample::InviteUsers()\n");
    m_multiplayerManager->lobby_session()->invite_friends(GetCurrentUser(), L"", L"");
}

void Sample::SearchJoinableUsers()
{
    DebugTrace("Sample::SearchJoinableUsers()\n");

    if (m_searching)
    {
        return;
    }

    m_searching = true;
    m_inLobby = false;
    m_selectPlayer = true;

    m_ui->FindPanel<ATG::Overlay>(400)->Show();
    m_ui->FindControl<ATG::TextLabel>(400, 102)->SetText(L"Looking for sessions...");

    m_xuidToHandle.clear();

    auto users = std::vector<std::shared_ptr<UserListItem>>();

    for (auto x = 0; x < MAXUSERS; x++)
    {
        users.push_back(std::make_shared<UserListItem>());
    }

    // Clear out the list first
    m_playerList->UpdateList(users);

    concurrency::create_task(
        m_context->multiplayer_service().get_activities_for_social_group(
            m_context->application_config()->scid(),
            GetCurrentUser()->xbox_user_id(),
            social::social_group_constants::people()
        )
    ).then([this](xbox_live_result<std::vector<multiplayer_activity_details>> t)
    {
        if (!t.err())
        {
            auto details = t.payload();
            auto list = std::vector<std::wstring>();

            for (auto detail : details)
            {
                bool joinableSession = detail.title_id() == m_context->application_config()->title_id()
                    && detail.members_count() < detail.max_members_count()
                    && !detail.closed()
                    && detail.join_restriction() != multiplayer::multiplayer_session_restriction::local
                    && detail.visibility() == multiplayer::multiplayer_session_visibility::open;

                if (joinableSession)
                {
                    list.push_back(detail.owner_xbox_user_id());
                    m_joinableXuids.push_back(detail.owner_xbox_user_id());
                    m_xuidToHandle[detail.owner_xbox_user_id()] = detail.handle_id();
                }
            }

            DebugTrace("Found %d available session(s).\n", list.size());

            if (list.size() > 0)
            {
                concurrency::create_task(
                    m_context->profile_service().get_user_profiles(list)
                    )
                .then([this](xbox_live_result<std::vector<social::xbox_user_profile>> t)
                {
                    if (!t.err())
                    {
                        // Update the ui list based on the profile results

                        auto profiles = t.payload();
                        auto users = std::vector<std::shared_ptr<UserListItem>>();

                        for (unsigned i = 0; i < MAXUSERS; i++)
                        {
                            if (i < profiles.size())
                            {
                                users.push_back(std::make_shared<UserListItem>(std::wstring(profiles[i].app_display_name())));
                            }
                            else
                            {
                                users.push_back(std::make_shared<UserListItem>());
                            }
                        }

                        if (profiles.size() > 0)
                        {
                            m_ui->FindControl<ATG::TextLabel>(400, 102)->SetText(L"Select session to join");
                        }
                        else
                        {
                            m_ui->FindControl<ATG::TextLabel>(400, 102)->SetText(L"No sessions found");
                        }

                        m_playerList->UpdateList(users);
                    }
                    else
                    {
                        m_ui->FindControl<ATG::TextLabel>(400, 102)->SetText(L"Unable to get profiles");
                    }

                    m_searching = false;
                });
            }
            else
            {
                m_ui->FindControl<ATG::TextLabel>(400, 102)->SetText(L"No sessions found");
                m_searching = false;
            }
        }
    });
}

void Sample::SetupChat()
{
    DebugTrace("Sample::SetupChat()\n");

    GetChatIntegrationLayer()->Initialize();
    GetChatIntegrationLayer()->AddLocalUser(GetCurrentUser()->xbox_user_id().c_str());

    m_inChat = true;
}

void Sample::LeaveChat()
{
    DebugTrace("Sample::LeaveChat()\n");

    GetChatIntegrationLayer()->RemoveLocalUser(GetCurrentUser()->xbox_user_id().c_str());
    GetChatIntegrationLayer()->ProcessDataFrames();
    GetChatIntegrationLayer()->Shutdown();
    ShutdownMeshManager();

    m_inChat = false;
}

void Sample::LoadUserDisplayNames()
{
    DebugTrace("Sample::LoadUserDisplayNames()\n");

    if (m_context == nullptr || m_needNames == false)
    {
        return;
    }

    m_needNames = false;
    m_loadingNames = true;

    auto list = std::vector<std::wstring>();

    for (auto user : GetChatIntegrationLayer()->GetChatUsersXuids())
    {
        list.push_back(user);
    }

    concurrency::create_task(
        m_context->profile_service().get_user_profiles(
            list
        )
    ).then([this](xbox_live_result<std::vector<social::xbox_user_profile>> t)
    {
        if (!t.err())
        {
            auto friendProfiles = t.payload();

            std::lock_guard<std::mutex> lock(m_stateLock);

            m_xuidToGamertag.clear();

            for (auto friendProfile : friendProfiles)
            {
                m_xuidToGamertag[friendProfile.xbox_user_id()] = friendProfile.game_display_name();
            }

            auto user = GetCurrentUser();

            m_xuidToGamertag[user->xbox_user_id()] = user->gamertag();
        }
        else
        {
            DebugTrace("get_user_profiles failed: %ws\n", t.err_message().c_str());
        }

        m_refreshTimer = 1.0f;
    });
}

void Sample::JoinSession()
{
    DebugTrace("Sample::JoinSession()\n");

    if (m_networkReady == false)
    {
        DebugTrace("Network not ready yet.\n");
        return;
    }

    auto user = GetCurrentUser();

    if (user == nullptr)
    {
        DebugTrace("No user bound to controller.\n");
        return;
    }

    m_multiplayerManager->lobby_session()->add_local_user(user);
    m_multiplayerManager->lobby_session()->set_local_member_connection_address(
        user,
        Windows::Networking::XboxLive::XboxLiveDeviceAddress::GetLocal()->GetSnapshotAsBase64()->Data(),
        nullptr
    );

    if (m_activationArgs != nullptr)
    {
        // Join based on protocol activation
        m_multiplayerManager->join_lobby(m_activationArgs, GetCurrentUser());

        m_activationArgs = nullptr;
    }
    else if (!m_joinXuid.empty())
    {
        // Join a friend's session
        m_multiplayerManager->join_lobby(m_xuidToHandle[m_joinXuid], GetCurrentUser());

        m_joinXuid.clear();
    }

    m_inLobby = false;
    m_ui->FindPanel<ATG::Overlay>(300)->Show();
}

void Sample::LeaveSession()
{
    DebugTrace("Sample::LeaveSession()\n");

    m_viewStats = true;
    m_inLobby = true;
    m_ui->FindPanel<ATG::Overlay>(200)->Show();
}

void Sample::SendTextMessage(Platform::String^ message)
{
    auto chatUser = GetChatIntegrationLayer()->GetChatUserByXboxUserId(GetCurrentUser()->xbox_user_id().c_str());

    chatUser->local()->send_chat_text(message->Data());

    if (chatUser->local()->text_to_speech_conversion_preference_enabled())
    {
        chatUser->local()->synthesize_text_to_speech(message->Data());
    }
}

void Sample::ChangeChannel(int updown)
{
    auto chatUser = GetChatIntegrationLayer()->GetChatUserByXboxUserId(GetCurrentUser()->xbox_user_id().c_str());
    auto channel = GetChatIntegrationLayer()->GetChannelForUser(chatUser->xbox_user_id());
    auto nchan = channel + updown;

    if (nchan > 2) nchan = 0;
    if (nchan < 0) nchan = 2;

    m_multiplayerManager->lobby_session()->set_local_member_properties(
        GetCurrentUser(),
        L"channel",
        web::json::value::number(nchan)
        );

    GetChatIntegrationLayer()->ChangeChannelForUser(
        chatUser->xbox_user_id(),
        static_cast<uint8>(nchan)
        );
}

#pragma endregion

void Sample::ProcessMpmMessages()
{
    auto queue = m_multiplayerManager->do_work();
    for (auto item : queue)
    {
        switch (item.event_type())
        {
        case multiplayer_event_type::arbitration_complete:
            DebugTrace("multiplayer_event_type::arbitration_complete\n");
            break;

        case multiplayer_event_type::client_disconnected_from_multiplayer_service:
            DebugTrace("multiplayer_event_type::client_disconnected_from_multiplayer_service\n");

            if (m_inChat)
            {
                LeaveChat();
            }

            LeaveSession();

            break;

        case multiplayer_event_type::find_match_completed:
            DebugTrace("multiplayer_event_type::find_match_completed\n");
            break;

        case multiplayer_event_type::host_changed:
            DebugTrace("multiplayer_event_type::host_changed\n");
            break;

        case multiplayer_event_type::invite_sent:
            DebugTrace("multiplayer_event_type::invite_sent\n");
            break;

        case multiplayer_event_type::joinability_state_changed:
            DebugTrace("multiplayer_event_type::joinability_state_changed\n");
            break;

        case multiplayer_event_type::join_game_completed:
            DebugTrace("multiplayer_event_type::join_game_completed\n");
            break;

        case multiplayer_event_type::join_lobby_completed:
        {
            DebugTrace("multiplayer_event_type::join_lobby_completed\n");

            for (auto member : m_multiplayerManager->lobby_session()->members())
            {
                if (member->is_local())
                {
                    InitializeMeshManager(static_cast<BYTE>(member->member_id()));
                    SetupChat();
                    break;
                }
            }

            break;
        }

        case multiplayer_event_type::leave_game_completed:
            DebugTrace("multiplayer_event_type::leave_game_completed\n");
            break;

        case multiplayer_event_type::local_member_connection_address_write_completed:
            DebugTrace("multiplayer_event_type::local_member_connection_address_write_completed\n");
            break;

        case multiplayer_event_type::local_member_property_write_completed:
            DebugTrace("multiplayer_event_type::local_member_property_write_completed\n");
            break;

        case multiplayer_event_type::member_joined:
            DebugTrace("multiplayer_event_type::member_joined\n");
            break;

        case multiplayer_event_type::member_left:
            DebugTrace("multiplayer_event_type::member_left\n");
            break;

        case multiplayer_event_type::member_property_changed:
        {
            DebugTrace("multiplayer_event_type::member_property_changed\n");

            auto propChangedArgs = std::dynamic_pointer_cast<manager::member_property_changed_event_args>(item.event_args());
            auto prop = propChangedArgs->properties()[L"channel"];

            GetChatIntegrationLayer()->ChangeChannelForUser(
                propChangedArgs->member()->xbox_user_id().c_str(),
                static_cast<uint8>(prop.as_integer())
                );

            break;
        }

        case multiplayer_event_type::perform_qos_measurements:
            DebugTrace("multiplayer_event_type::perform_qos_measurements\n");
            break;

        case multiplayer_event_type::session_property_changed:
            DebugTrace("multiplayer_event_type::session_property_changed\n");
            break;

        case multiplayer_event_type::session_property_write_completed:
            DebugTrace("multiplayer_event_type::session_property_write_completed\n");
            break;

        case multiplayer_event_type::session_synchronized_property_write_completed:
            DebugTrace("multiplayer_event_type::session_synchronized_property_write_completed\n");
            break;

        case multiplayer_event_type::synchronized_host_write_completed:
            DebugTrace("multiplayer_event_type::synchronized_host_write_completed\n");
            break;

        case multiplayer_event_type::user_added:
        {
            DebugTrace("multiplayer_event_type::user_added\n");

            auto userAddedArgs = std::dynamic_pointer_cast<manager::user_added_event_args>(item.event_args());

            for (auto member : m_multiplayerManager->lobby_session()->members())
            {
                if (userAddedArgs->xbox_user_id() == member->xbox_user_id() && member->is_local())
                {
                    if (m_meshManager == nullptr)
                    {
                        InitializeMeshManager(static_cast<BYTE>(member->member_id()));
                    }

                    SetupChat();
                }
            }

            break;
        }

        case multiplayer_event_type::user_removed:
        {
            DebugTrace("multiplayer_event_type::user_removed\n");

            auto userRemovedArgs = std::dynamic_pointer_cast<manager::user_removed_event_args>(item.event_args());

            if (userRemovedArgs != nullptr)
            {
                if (m_inChat)
                {
                    LeaveChat();
                }

                if (GetCurrentUser()->xbox_user_id() == userRemovedArgs->xbox_user_id())
                {
                    LeaveSession();
                }
            }

            m_leavingToJoin = false;

            break;
        }

        default:
            DebugTrace("Unkonwn multiplayer_event_type encountered: %d\n", static_cast<int>(item.event_type()));
            break;
        }
    }

    GetChatIntegrationLayer()->ProcessDataFrames();
}

#pragma region NetworkHandling
void Sample::InitializeMeshManager(BYTE consoleID)
{
    DebugTrace("Sample::InitializeMeshManager(%d)\n", consoleID);

    if (m_meshInitialized)
    {
        return;
    }

    // Initialize winsock and the network layer
    try
    {
        m_meshManager = ref new Microsoft::Xbox::Samples::NetworkMesh::MeshManager(
            consoleID,
            SECURE_DEVICE_ASSOCIATION_TEMPLATE_UDP,
            Utils::GetDebugNameForLocalConsole(),
            false  // drop out-of-order packets
            );

        // Register event handlers that are fired when various network packets arrive
        RegisterMeshControllerEventHandlers();

        if (m_multiplayerManager != nullptr &&
            m_multiplayerManager->lobby_session() != nullptr)
        {
            // Connect to peers
            for (auto member : m_multiplayerManager->lobby_session()->members())
            {
                if (member->is_local() || member->connection_address().empty())
                {
                    continue;
                }

                m_meshManager->ConnectToAddress(
                    Windows::Networking::XboxLive::XboxLiveDeviceAddress::CreateFromSnapshotBase64(ref new Platform::String(member->connection_address().c_str())),
                    ref new Platform::String(member->debug_gamertag().c_str())
                    );
            }
        }

        m_meshInitialized = true;
    }
    catch (Platform::Exception^ ex)
    {
        DebugTrace("Exception creating mesh: 0x%x\n", ex->HResult);
    }
}

void Sample::ShutdownMeshManager()
{
    DebugTrace("Sample::ShutdownMeshManager()\n");

    if (m_meshManager != nullptr)
    {
        m_meshManager->GetMeshPacketManager()->OnChatMessageReceived -= m_chatToken;
        m_meshManager->OnDebugMessage -= m_debugToken;
        m_meshManager->OnPostHandshake -= m_handshakeToken;
        m_meshManager->OnDisconnected -= m_disconnectToken;

        m_meshManager->Shutdown();
        m_meshManager = nullptr;

        m_meshInitialized = false;
    }
}

void Sample::RegisterMeshControllerEventHandlers()
{
    DebugTrace("Sample::RegisterMeshControllerEventHandlers()\n");

    m_debugToken = m_meshManager->OnDebugMessage += ref new EventHandler<Microsoft::Xbox::Samples::NetworkMesh::DebugMessageEventArgs^>(
        [this](Platform::Object^, Microsoft::Xbox::Samples::NetworkMesh::DebugMessageEventArgs^ args)
    {
        DebugTrace("MeshManager::OnDebugMessage()\n");
        LogToConsole(args->Message);
    });

    m_handshakeToken = m_meshManager->OnPostHandshake += ref new EventHandler<MeshConnection^>(
        [this](Platform::Object^, MeshConnection^ args)
    {
        DebugTrace("MeshManager::OnPostHandshake()\n");

        if (m_multiplayerManager)
        {
            std::vector<std::wstring> xuids;

            auto members = m_multiplayerManager->lobby_session()->members();

            for (auto member : members)
            {
                if (!member->is_local() && member->connection_address() == args->GetSecureDeviceAddress()->GetSnapshotAsBase64()->Data())
                {
                    xuids.push_back(member->xbox_user_id().c_str());
                }
            }

            GetChatIntegrationLayer()->OnSecureDeviceAssocationConnectionEstablished(args, xuids);
        }
        else
        {
            DebugTrace("MultiplayerManager is null\n");
        }
    });

    m_disconnectToken = m_meshManager->OnDisconnected += ref new EventHandler<MeshConnection^>(
        [this](Platform::Object^, MeshConnection^ args)
    {
        DebugTrace("MeshManager::OnDisconnected()\n");
        GetChatIntegrationLayer()->RemoveRemoteConsole(args->GetRemoteId());
    });

    m_chatToken = m_meshManager->GetMeshPacketManager()->OnChatMessageReceived += ref new EventHandler<MeshChatMessageReceivedEvent^>(
        [this](Platform::Object^, MeshChatMessageReceivedEvent^ args)
    {
        Windows::Storage::Streams::IBuffer^ chatPacket = args->Buffer;
        GetChatIntegrationLayer()->OnIncomingChatMessage(chatPacket, args->Sender->GetRemoteId());
    });
}
#pragma endregion

#pragma region Helpers
std::wstring Sample::GetNameFromXuid(std::wstring xuid)
{
    std::lock_guard<std::mutex> lock(m_stateLock);

    auto it = m_xuidToGamertag.find(xuid);
    if (it != std::end(m_xuidToGamertag))
    {
        return (*it).second;
    }

    ReloadDisplayNames();

    return xuid;
}

void Sample::ReloadDisplayNames()
{
    m_needNames = true;
}

void Sample::LogToConsole(Platform::String ^ val)
{
    if (m_console)
    {
        m_console->Write(val->Data());
    }
}

std::wstring Sample::FormatNumber(double val)
{
    if (val == 100000.0)
    {
        val = 0.0;
    }

    wchar_t buffer[32] = {};
    int intval = (int)val;

    swprintf_s(buffer, 32, L"%02d.%02d", intval, (int)((val - (double)intval) * 100));

    return std::wstring(buffer);
}
#pragma endregion
