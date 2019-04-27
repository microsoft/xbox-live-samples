// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "pch.h"
#include "SampleGame.h"

#include "DirectXHelper.h"
#include "GameSave.h"

using namespace Concurrency;
using namespace DirectX;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;

GameSaveSample::SampleGame* g_gameInstance = nullptr;

namespace GameSaveSample {

// Loads and initializes application assets when the application is loaded.
SampleGame::SampleGame() :
    m_gamepadIndex(-1)
{
    g_gameInstance = this;

    srand(unsigned int(time(nullptr)));

    m_deviceResources = std::make_shared<DX::DeviceResources>(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_UNKNOWN);
#ifndef _XBOX_ONE
    // Register to be notified if the Device is lost or recreated
    m_deviceResources->RegisterDeviceNotify(this);
#endif

    ScreenManager = std::make_shared<GameSaveSample::ScreenManager>();
    StateManager = ref new GameSaveSample::StateManager(ScreenManager);

    LiveResources = std::make_shared<ATG::LiveResources>(false, true);

#ifdef _XBOX_ONE
    LiveResources->SetUserSignOutStartedCallback([this](Windows::Xbox::System::SignOutStartedEventArgs^ args)
    {
        this->StateManager->OnSignOutStarted(args);
    });
#else // UWP
    LiveResources->SetUserSignOutCompletedCallback([this](ATG::XboxLiveUser user)
    {
        this->StateManager->OnSignOutCompleted(user);
    });
#endif
}

// Initialize the Direct3D resources required to run.
#ifdef _XBOX_ONE
void SampleGame::Initialize(IUnknown* window)
{
    m_deviceResources->SetWindow(window);

    PlatformIndependentInitialize(window);

    Windows::Xbox::Input::Controller::ControllerPairingChanged += ref new EventHandler<Windows::Xbox::Input::ControllerPairingChangedEventArgs^>(this->StateManager, &GameSaveSample::StateManager::OnControllerPairingChanged);

    Windows::Xbox::Input::Controller::ControllerRemoved += ref new EventHandler<Windows::Xbox::Input::ControllerRemovedEventArgs^>(this->StateManager, &GameSaveSample::StateManager::OnControllerRemoved);
}
#else // UWP
void SampleGame::Initialize(IUnknown* window, int width, int height, DXGI_MODE_ROTATION rotation)
{
    m_deviceResources->SetWindow(window, width, height, rotation);

    PlatformIndependentInitialize(window);

    if (Windows::System::Profile::AnalyticsInfo::VersionInfo->DeviceFamily == "Windows.Xbox")
    {
        // SignOutStarted equivalent for UWP on Xbox
        m_userWatcher = Windows::System::User::CreateWatcher();
        m_userWatcher->Added += ref new Windows::Foundation::TypedEventHandler<Windows::System::UserWatcher^, Windows::System::UserChangedEventArgs^>(&GameSaveSample::StateManager::OnAdded);
        m_userWatcher->AuthenticationStatusChanging += ref new Windows::Foundation::TypedEventHandler<Windows::System::UserWatcher^, Windows::System::UserAuthenticationStatusChangingEventArgs^>(this->StateManager, &GameSaveSample::StateManager::OnAuthenticationStatusChanging);
    }
    else
    {
        create_task(Windows::System::User::FindAllAsync()).then([this](IVectorView<Windows::System::User^>^ users)
        {
            if (users->Size > 0)
            {
                LiveResources->SetWindowsUser(users->GetAt(0));
            }
            else
            {
                Log::Write("ERROR: No Windows System User found\n");
            }
        });
    }
}
#endif

void SampleGame::PlatformIndependentInitialize(IUnknown* window)
{
    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    InputManager = std::make_unique<InputState>();
    InputManager->SetWindow(window);

    LiveResources->Initialize();

    GameSaveManager = ref new GameSaveSample::GameSaveManager();
}

void SampleGame::CreateDeviceDependentResources()
{
#ifdef _XBOX_ONE
    m_graphicsMemory = std::make_unique<GraphicsMemory>(m_deviceResources->GetD3DDevice(), m_deviceResources->GetBackBufferCount());
#endif

    ScreenManager->CreateDeviceDependentResources(m_deviceResources);
}

// Updates application state when the window size changes (e.g. device orientation change)
void SampleGame::CreateWindowSizeDependentResources()
{
    ScreenManager->CreateWindowSizeDependentResources();
}

// Executes basic game loop
void SampleGame::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

// Updates the application state once per frame
void SampleGame::Update(DX::StepTimer const& timer)
{
    PIXBeginEvent(EVT_COLOR_UPDATE, L"Update");

    // Update scene objects
    this->InputManager->Update();
    this->ScreenManager->Update(timer);
    this->StateManager->Update();

    PIXEndEvent();
}

// Helper method to clear the backbuffers
void SampleGame::Clear()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    PIXBeginEvent(context, EVT_COLOR_CLEAR, L"Clear");

    // Clear the views
    auto renderTarget = m_deviceResources->GetRenderTargetView();

    XMVECTORF32 backgroundColor = Colors::Black;
    context->ClearRenderTargetView(renderTarget, backgroundColor);
    context->OMSetRenderTargets(1, &renderTarget, nullptr);

    // Set the viewport
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    PIXEndEvent(context);
}

// Renders the current frame according to the current application state
void SampleGame::Render()
{
    // Don't try to render anything before the first Update
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

#ifdef _XBOX_ONE
    // Prepare the render target to render a new frame.
    m_deviceResources->Prepare();
#endif

    Clear();

    auto context = m_deviceResources->GetD3DDeviceContext();
    PIXBeginEvent(context, EVT_COLOR_RENDER, L"Render");

    // Render the scene objects
    ScreenManager->Render(m_timer);

    PIXEndEvent(context);

    // Present the contents of the swap chain to the screen
    PIXBeginEvent(context, EVT_COLOR_PRESENT, L"Present");

    m_deviceResources->Present();

#ifdef _XBOX_ONE
    m_graphicsMemory->Commit();
#endif

    PIXEndEvent(context);
}

#ifndef _XBOX_ONE
// Notifies renderers that device resources need to be released.
void SampleGame::OnDeviceLost()
{
    Log::Write("OnDeviceLost()\n");
    ScreenManager->OnDeviceLost();
}

// Notifies renderers that device resources may now be recreated.
void SampleGame::OnDeviceRestored()
{
    Log::Write("OnDeviceRestored()\n");
    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}
#endif

void SampleGame::OnSuspending(Windows::ApplicationModel::SuspendingDeferral^ deferral)
{
    Log::Write("OnSuspending()\n");

    auto context = m_deviceResources->GetD3DDeviceContext();

#ifdef _XBOX_ONE
    context->Suspend(0);
#else
    context->ClearState();
    m_deviceResources->Trim();
#endif

    StateManager->Suspend(deferral);
    InputManager->Suspend();
}

void SampleGame::OnResuming()
{
    Log::Write("OnResuming()\n");

#ifdef _XBOX_ONE
    auto context = m_deviceResources->GetD3DDeviceContext();
    context->Resume();
#endif

    InputManager->Resume();
    StateManager->Resume();
    m_timer.ResetElapsedTime();
}

#ifndef _XBOX_ONE
void SampleGame::OnKeyUp(Windows::UI::Core::KeyEventArgs^ args)
{
    InputManager->OnKeyUp(args);
}

void SampleGame::OnWindowActivated()
{
    Log::WriteAndDisplay("OnVisibilityChanged to visible\n");
}

void SampleGame::OnWindowDeactivated()
{
    Log::WriteAndDisplay("OnVisibilityChanged to NOT visible\n");
}

void SampleGame::OnWindowSizeChanged(int width, int height, DXGI_MODE_ROTATION rotation, float dpi)
{
    InputManager->SetDpi(dpi);

    if (m_deviceResources->WindowSizeChanged(width, height, rotation))
    {
        CreateWindowSizeDependentResources();
    }

    auto outputSize = m_deviceResources->GetOutputSize();
    auto aspectRatio = float(outputSize.right) / float(outputSize.bottom);
    Log::Write("OnWindowSizeChanged() -> %d x %d (a/r = %.2f); DPI is %d\n", outputSize.right, outputSize.bottom, aspectRatio, lround(dpi));
}

void SampleGame::ValidateDevice()
{
    m_deviceResources->ValidateDevice();
}
#endif

void SampleGame::GetDefaultSize(int& width, int& height) const
{
#ifdef _XBOX_ONE
    width = 1920;
    height = 1080;
#else // UWP
    width = 1280;
    height = 720;
#endif
}

task<HRESULT> SampleGame::InitializeGameSaveSystem()
{
    Log::Write("SampleGame::InitializeGameSaveSystem()\n");

#ifdef _XBOX_ONE
    if (LiveResources->IsUserSignedIn())
    {
        auto xblUser = LiveResources->GetUser();
        return this->GameSaveManager->InitializeForUser(xblUser, false);
    }
    else
    {
        Log::WriteAndDisplay("ERROR initializing game save system: Xbox Live User not signed in\n");
        return create_task([] { return E_FAIL; });
    }

#else // UWP
    if (!LiveResources->IsUserSignedIn())
    {
        Log::WriteAndDisplay("ERROR initializing game save system: Xbox Live User not signed in\n");
        return create_task([] { return E_FAIL; });
    }

    auto winUser = LiveResources->GetWindowsUser();
    if (winUser == nullptr)
    {
        Log::WriteAndDisplay("ERROR initializing game save system: Windows System User not found\n");
        return create_task([] { return E_FAIL; });
    }

    auto xblUser = LiveResources->GetUser();
    Platform::String^ scid = ref new Platform::String(LiveResources->GetServiceConfigId().c_str());
    return this->GameSaveManager->InitializeForUser(winUser, xblUser, scid, false);
#endif
}

#ifdef _XBOX_ONE
void SampleGame::SetCurrentGamepad(Windows::Xbox::Input::IGamepad^ gamepad)
{
    m_gamepad = gamepad;
    m_gamepadIndex = (gamepad == nullptr) ? -1 : InputManager->GetGamepadIndex(gamepad);
    if (ScreenManager != nullptr)
    {
        ScreenManager->UpdateControllingPlayer(m_gamepadIndex);
    }
}
#else
void SampleGame::SetCurrentGamepad(Windows::Gaming::Input::IGamepad^ gamepad)
{
    m_gamepad = gamepad;
    m_gamepadIndex = -1;
    if (ScreenManager != nullptr)
    {
        ScreenManager->UpdateControllingPlayer(m_gamepadIndex);
    }
}
#endif

bool SampleGame::UpdateCurrentGamepad()
{
#ifdef _XBOX_ONE
    auto xblUser = LiveResources->GetUser();
    if (xblUser == nullptr)
    {
        SetCurrentGamepad(nullptr);
        return false;
    }

    if (m_gamepad != nullptr)
    {
        // is the active user already paired with the current gamepad?
        if (m_gamepad->User != nullptr && (m_gamepad->User->Id == xblUser->Id))
        {
            SetCurrentGamepad(m_gamepad);
            Log::WriteAndDisplay("Gamepad refreshed for current user (index = %d)\n", m_gamepadIndex);
            return true;
        }
    }

    // is the active user paired with another gamepad?
    for (const auto& gamepad : Windows::Xbox::Input::Gamepad::Gamepads)
    {
        if (gamepad->User != nullptr && gamepad->User->Id == xblUser->Id)
        {
            SetCurrentGamepad(gamepad);
            Log::WriteAndDisplay("New gamepad set for current user (index = %d)\n", m_gamepadIndex);
            return true;
        }
    }

    SetCurrentGamepad(nullptr);
    return false;
#else
    SetCurrentGamepad(nullptr);
    return true;
#endif
}

} // namespace GameSaveSample
