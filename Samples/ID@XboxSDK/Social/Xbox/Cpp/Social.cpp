// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "pch.h"
#include "Social.h"

#include "ATGColors.h"

using namespace DirectX;
using namespace xbox::services::social::manager;
using namespace Windows::Xbox::System;

using Microsoft::WRL::ComPtr;

namespace
{
    const int c_liveHUD = 1000;
    const int c_sampleUIPanel = 2000;
    const int c_openFriendListBtn = 2101;
    const int c_friendListPopup = 3000;
    const int c_friendListBox = 3001;
    const int c_friendListFilterBtn = 3100;
    const int c_friendListQuitBtn = 3101;

    std::wstring friendListTypeStrings[] =
    {
        L"All Friends",
        L"All Online Friends",
        L"All Online In-Title Friends",
        L"All Favorites",
        L"Custom"
    };
}

Sample::Sample() :
    m_frame(0)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_liveResources = std::make_shared<ATG::LiveResources>();
    m_liveInfoHud = std::make_unique<ATG::LiveInfoHUD>(L"Social Sample");

    ATG::UIConfig uiconfig;
    m_ui = std::make_shared<ATG::UIManager>(uiconfig);
}

// Initialize the Direct3D resources required to run.
void Sample::Initialize(IUnknown* window)
{
    m_gamePad = std::make_unique<GamePad>();

    m_ui->LoadLayout(L".\\Assets\\SampleUI.csv", L".\\Assets");

    //m_liveResources->Initialize();
    m_deviceResources->SetWindow(window);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    // Set the callbacks before initializing the LiveInfoHUD
    m_liveResources->SetUserChangedCallback([this](ATG::XboxLiveUser user)
    {
        m_liveInfoHud->SetUser(m_liveResources->GetLiveContext());
        m_ui->FindPanel<ATG::IPanel>(200)->Show();
    });

    m_liveResources->SetUserSignOutCompletedCallback([this](ATG::XboxLiveUser)
    {
        m_liveInfoHud->SetUser(m_liveResources->GetLiveContext());
        m_ui->FindPanel<ATG::IPanel>(200)->Close();
    });

    // Initialize these after the device and window dependant resources
    m_liveResources->Initialize();
    m_liveInfoHud->Initialize(m_liveResources->GetLiveContext());

    SetupUI();

    auto user = m_liveResources->GetUser();

    if (user != nullptr)
    {
        InitializeSocialManager(user);
    }

    User::UserAdded += ref new Windows::Foundation::EventHandler<UserAddedEventArgs^>(
        [this](Platform::Object^, UserAddedEventArgs^ eventArgs)
    {
        if (m_socialManager == nullptr)
        {
            auto user = m_liveResources->GetUser();

            if (user != nullptr)
            {
                InitializeSocialManager(user);
            }

        }
    });
}

#pragma region UI Methods
void Sample::SetupUI()
{
    using namespace ATG;

    m_selectedFriendList = friendListType::allFriends;

    // Setup user list repeater
    auto loc = POINT{ 150, 200 };
    auto pos = SIZE{ 610, 40 };

    m_userList = std::make_unique<UserRepeater>(m_ui, loc, pos, 8000);
    m_userList->SetSelectedCallback([this](unsigned index)
    {
        if (index >= m_socialGroups[m_selectedFriendList]->users().size())
            return;

        auto user = m_socialGroups[m_selectedFriendList]->users().at(index);
        stringstream_t source;
        source << _T("\nDisplaying info about user ") << index << _T(":");
        source << _T("\n   display_name: ");
        source << user->display_name();
        source << _T("\n   url: ");
        string_t url = user->display_pic_url_raw();
        url.resize(35);
        source << url << "...";
        source << _T("\n   gamerscore: ");
        source << user->gamerscore();
        source << _T("\n   gamertag: ");
        source << user->gamertag();
        source << _T("\n   is_favorite: ");
        source << user->is_favorite();
        source << _T("\n   is_followed_by_caller: ");
        source << user->is_followed_by_caller();
        source << _T("\n   is_following_user: ");
        source << user->is_following_user();
        source << _T("\n   preferred_color: ");
        source << user->preferred_color().primary_color();
        source << _T("\n   real_name: ");
        source << user->real_name();
        source << _T("\n   use_avatar: ");
        source << user->use_avatar();
        source << _T("\n   has_user_played: ");
        source << user->title_history().has_user_played();
        source << _T("\n   last_time_user_played: ");
        source << user->title_history().last_time_user_played().to_string();

        int i = 0;
        for (auto const record : user->presence_record().presence_title_records())
        {
            source << _T("\n   presence_record[") << i << _T("]: ");
            source << record.title_id();
            source << _T("\n      title_id: ");
            source << record.title_id();
            source << _T("\n      is_title_active: ");
            source << record.is_title_active();
            source << _T("\n      presence_text: ");
            source << record.presence_text();
            source << _T("\n      is_broadcasting: ");
            source << record.is_broadcasting();
            i++;
        }

        m_console->WriteLine(source.str().c_str());
    });

    // Populate each list with empty items to start
    auto users = std::vector<std::shared_ptr<UserListItem>>();

    for (auto x = 0; x < 15; x++)
    {
        users.push_back(std::make_shared<UserListItem>(nullptr));
    }

    m_userList->GenerateList(200, users, 2);
}

void Sample::RefreshUserList()
{
    const auto& group = m_socialGroups[m_selectedFriendList];

    stringstream_t source;
    source << _T("Displaying ");
    source << friendListTypeStrings[m_selectedFriendList];
    source << _T(" with ");
    source << group->users().size();
    source << _T(" players.");
    m_console->WriteLine(source.str().c_str());

    auto users = std::vector<std::shared_ptr<UserListItem>>();
    const auto& userList = group->users();

    for (auto x = 0; x < 15; x++)
    {
        if (x < userList.size())
        {
            users.push_back(std::make_shared<UserListItem>(userList.at(x)));
        }
        else
        {
            users.push_back(std::make_shared<UserListItem>(nullptr));
        }
    }

    std::wstring label = L"Social Group: ";
    label += friendListTypeStrings[m_selectedFriendList];

    m_ui->FindControl<ATG::TextLabel>(200, 201)->SetText(label.c_str());
    m_userList->UpdateList(users);
}
#pragma endregion

#pragma region Frame Update
// Executes basic render loop.
void Sample::Tick()
{
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Frame %I64u", m_frame);

    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();

    PIXEndEvent();
    m_frame++;
}

// Updates the world.
void Sample::Update(DX::StepTimer const& timer)
{
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update");

    float elapsedTime = float(timer.GetElapsedSeconds());
    auto refresh = false;
    auto pad = m_gamePad->GetState(0);

    if (pad.IsConnected())
    {
        m_gamePadButtons.Update(pad);

        m_ui->Update(elapsedTime, pad);

        if (pad.IsViewPressed() || pad.IsBPressed())
        {
            Windows::ApplicationModel::Core::CoreApplication::Exit();
        }
        if (pad.IsMenuPressed())
        {
            Windows::Xbox::UI::SystemUI::ShowAccountPickerAsync(nullptr, Windows::Xbox::UI::AccountPickerOptions::None);
        }
        if (m_gamePadButtons.rightShoulder == GamePad::ButtonStateTracker::PRESSED)
        {
            int newVal = ((int)m_selectedFriendList) + 1;

            if (newVal > 3)
            {
                newVal = 0;
            }

            m_selectedFriendList = (friendListType)newVal;
            refresh = true;
        }
        if (m_gamePadButtons.leftShoulder == GamePad::ButtonStateTracker::PRESSED)
        {
            int newVal = ((int)m_selectedFriendList) - 1;

            if (newVal < 0)
            {
                newVal = 3;
            }

            m_selectedFriendList = (friendListType)newVal;
            refresh = true;
        }
    }
    else
    {
        m_gamePadButtons.Reset();
    }

    if (refresh)
    {
        std::lock_guard<std::mutex> guard(m_socialManagerLock);
        RefreshUserList();
    }

    if (m_socialManager != nullptr)
    {
        std::lock_guard<std::mutex> guard(m_socialManagerLock);
        UpdateSocialManager();
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

    // Prepare the render target to render a new frame.
    m_deviceResources->Prepare();
    Clear();

    auto context = m_deviceResources->GetD3DDeviceContext();
    PIXBeginEvent(context, PIX_COLOR_DEFAULT, L"Render");
    m_liveInfoHud->Render();
    m_ui->Render();
    m_console->Render();
    PIXEndEvent(context);

    // Show the new frame.
    PIXBeginEvent(context, PIX_COLOR_DEFAULT, L"Present");
    m_deviceResources->Present();
    m_graphicsMemory->Commit();
    PIXEndEvent(context);
}

// Helper method to clear the back buffers.
void Sample::Clear()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    PIXBeginEvent(context, PIX_COLOR_DEFAULT, L"Clear");

    // Clear the views.
    auto renderTarget = m_deviceResources->GetBackBufferRenderTargetView();
    context->ClearRenderTargetView(renderTarget, ATG::Colors::Background);
    context->OMSetRenderTargets(1, &renderTarget, nullptr);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    PIXEndEvent(context);
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Sample::OnSuspending()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    context->Suspend(0);
}

void Sample::OnResuming()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    context->Resume();
    m_timer.ResetElapsedTime();
    m_gamePadButtons.Reset();
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Sample::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();

    m_graphicsMemory = std::make_unique<GraphicsMemory>(device, m_deviceResources->GetBackBufferCount());
    m_console = std::make_unique<DX::TextConsole>(context, L"Courier_16.spritefont");
    m_liveInfoHud->RestoreDevice(context);

    m_ui->RestoreDevice(context);
}

// Allocate all memory resources that change on a window SizeChanged event.
void Sample::CreateWindowSizeDependentResources()
{
    RECT fullscreen = m_deviceResources->GetOutputSize();

    m_ui->SetWindow(fullscreen);

    RECT console = { 0 };

    console.top = 200;
    console.left = 850;
    console.bottom = console.top + 700;
    console.right = console.left + 700;

    m_console->SetWindow(console);
}
#pragma endregion
