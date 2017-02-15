//--------------------------------------------------------------------------------------
// Social.cpp
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "Social.h"

#include "ATGColors.h"

using namespace DirectX;
using namespace xbox::services::social::manager;

using Microsoft::WRL::ComPtr;

namespace
{
    const int c_userList = 2000;
    const int c_debugLog = 202;

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

    ATG::UIConfig uiconfig;
    m_ui = std::make_shared<ATG::UIManager>(uiconfig);
}

void Sample::HandleSignin(
    _In_ std::shared_ptr<xbox::services::system::xbox_live_user> user,
    _In_ xbox::services::system::sign_in_status result
)
{
    if (result == xbox::services::system::sign_in_status::success)
    {
        AddUserToSocialManager(user);
    }
}

void Sample::HandleSignout(_In_ std::shared_ptr<xbox::services::system::xbox_live_user> user)
{
    RemoveUserFromSocialManager(user);
}

// Initialize the Direct3D resources required to run.
void Sample::Initialize(IUnknown* window, int width, int height, DXGI_MODE_ROTATION rotation)
{
    m_gamePad = std::make_unique<GamePad>();

    m_keyboard = std::make_unique<Keyboard>();
    m_keyboard->SetWindow(reinterpret_cast<ABI::Windows::UI::Core::ICoreWindow*>(window));

    m_mouse = std::make_unique<Mouse>();
    m_mouse->SetWindow(reinterpret_cast<ABI::Windows::UI::Core::ICoreWindow*>(window));

    m_ui->LoadLayout(L".\\Assets\\SampleUI.csv", L".\\Assets");
    m_liveResources->Initialize(m_ui, m_ui->FindPanel<ATG::Overlay>(c_userList));

    m_deviceResources->SetWindow(window, width, height, rotation);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    SetupUI();

    InitializeSocialManager();

    std::weak_ptr<Sample> thisWeakPtr = shared_from_this();
    m_signInContext = m_liveResources->add_signin_handler([thisWeakPtr](
        _In_ std::shared_ptr<xbox::services::system::xbox_live_user> user,
        _In_ xbox::services::system::sign_in_status result
        )
    {
        std::shared_ptr<Sample> pThis(thisWeakPtr.lock());
        if (pThis != nullptr)
        {
            pThis->HandleSignin(user, result);
        }
    });

    m_signOutContext = xbox::services::system::xbox_live_user::add_sign_out_completed_handler(
        [thisWeakPtr](const xbox::services::system::sign_out_completed_event_args& args)
    {
        UNREFERENCED_PARAMETER(args);
        std::shared_ptr<Sample> pThis(thisWeakPtr.lock());
        if (pThis != nullptr)
        {
            pThis->HandleSignout(args.user());
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
        for(auto const record : user->presence_record().presence_title_records())
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

    m_userList->GenerateList(c_userList, users, 2);
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

    m_ui->FindControl<ATG::TextLabel>(c_userList, 201)->SetText(label.c_str());
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
    auto toggleRight = false;
    auto toggleLeft = false;
    auto pad = m_gamePad->GetState(0);

    if (pad.IsConnected())
    {
        m_gamePadButtons.Update(pad);

        m_ui->Update(elapsedTime, pad);

        if (pad.IsViewPressed() || pad.IsBPressed())
        {
            Windows::ApplicationModel::Core::CoreApplication::Exit();
        }
        if (m_gamePadButtons.rightShoulder == GamePad::ButtonStateTracker::PRESSED)
        {
            toggleRight = true;
        }
        if (m_gamePadButtons.leftShoulder == GamePad::ButtonStateTracker::PRESSED)
        {
            toggleLeft = true;
        }
    }
    else
    {
        m_gamePadButtons.Reset();
    }

    auto mouse = m_mouse->GetState();
    mouse;

    auto kb = m_keyboard->GetState();
    m_keyboardButtons.Update(kb);

    if (!m_ui->Update(elapsedTime, *m_mouse, *m_keyboard))
    {
        // UI is not consuming input, so implement sample mouse & keyboard controls
    }

    if (kb.Escape)
    {
        Windows::ApplicationModel::Core::CoreApplication::Exit();
    }

    if (m_keyboardButtons.IsKeyPressed(Keyboard::A))
    {
        m_liveResources->SignIn();
    }

    if (m_keyboardButtons.IsKeyPressed(Keyboard::Y))
    {
        m_liveResources->SwitchAccount();
    }

    if (m_keyboardButtons.IsKeyPressed(Keyboard::Left))
    {
        toggleLeft = true;
    }

    if (m_keyboardButtons.IsKeyPressed(Keyboard::Right))
    {
        toggleRight = true;
    }

    if (toggleLeft)
    {
        int newVal = ((int)m_selectedFriendList) - 1;

        if (newVal < 0)
        {
            newVal = 3;
        }

        m_selectedFriendList = (friendListType)newVal;

        std::lock_guard<std::mutex> guard(m_socialManagerLock);
        RefreshUserList();
    }

    if (toggleRight)
    {
        int newVal = ((int)m_selectedFriendList) + 1;

        if (newVal > 3)
        {
            newVal = 0;
        }

        m_selectedFriendList = (friendListType)newVal;

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

    Clear();

    auto context = m_deviceResources->GetD3DDeviceContext();
    PIXBeginEvent(context, PIX_COLOR_DEFAULT, L"Render");
    m_ui->Render();
    m_console->Render();
    PIXEndEvent(context);

    // Show the new frame.
    PIXBeginEvent(context, PIX_COLOR_DEFAULT, L"Present");
    m_deviceResources->Present();
    PIXEndEvent(context);
}

// Helper method to clear the back buffers.
void Sample::Clear()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    PIXBeginEvent(context, PIX_COLOR_DEFAULT, L"Clear");

    // Clear the views.
    auto renderTarget = m_deviceResources->GetBackBufferRenderTargetView();
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
void Sample::OnSuspending()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    context->ClearState();

    m_deviceResources->Trim();
}

void Sample::OnResuming()
{
    m_timer.ResetElapsedTime();
    m_gamePadButtons.Reset();
    m_keyboardButtons.Reset();

    // Reset UI on return from suspend.
    m_ui->Reset();
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Sample::CreateDeviceDependentResources()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    m_console = std::make_unique<DX::TextConsole>(context, L"SegoeUI_18.spritefont");

    // Let UI create Direct3D resources.
    m_ui->RestoreDevice(context);
}

// Allocate all memory resources that change on a window SizeChanged event.
void Sample::CreateWindowSizeDependentResources()
{
    // Let UI know the size of our rendering viewport.
    RECT fullscreen = m_deviceResources->GetOutputSize();
    m_ui->SetWindow(fullscreen);

    const RECT* label = m_ui->FindControl<ATG::Image>(c_userList, c_debugLog)->GetRectangle();

    RECT console = { 0 };
    console.top = label->top;
    console.left = label->left;
    console.bottom = console.top + 600;
    console.right = console.left + 800;

    m_console->SetWindow(console);
}

void Sample::OnDeviceLost()
{
    m_ui->ReleaseDevice();
}

void Sample::OnActivated()
{
}

void Sample::OnDeactivated()
{
}

void Sample::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}

void Sample::OnWindowSizeChanged(int width, int height, DXGI_MODE_ROTATION rotation)
{
    if (!m_deviceResources->WindowSizeChanged(width, height, rotation))
        return;

    CreateWindowSizeDependentResources();
}

void Sample::ValidateDevice()
{
    m_deviceResources->ValidateDevice();
}

void Sample::GetDefaultSize(int& width, int& height) const
{
    width = 1600;
    height = 1050;
}
#pragma endregion
