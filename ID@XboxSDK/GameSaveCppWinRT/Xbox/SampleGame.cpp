//--------------------------------------------------------------------------------------
// SampleGame.cpp
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "SampleGame.h"

#include "DirectXHelper.h"
#include "GameSave.h"

using namespace Concurrency;
using namespace DirectX;
using namespace winrt::Windows::Foundation::Collections;

GameSaveSample::SampleGame* g_gameInstance = nullptr;

namespace GameSaveSample {

// Loads and initializes application assets when the application is loaded.
SampleGame::SampleGame() :
    m_gamepadIndex(-1)
{
    g_gameInstance = this;

    srand(unsigned int(time(nullptr)));

    m_deviceResources = std::make_shared<DX::DeviceResources>(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_UNKNOWN);

    ScreenManager = std::make_shared<GameSaveSample::ScreenManager>();
    StateManager = new GameSaveSample::StateManager(ScreenManager);

    LiveResources = std::make_shared<ATG::LiveResources>(false, true);

    LiveResources->SetUserSignOutStartedCallback([this](winrt::Windows::Xbox::System::SignOutStartedEventArgs const & args)
    {
        this->StateManager->OnSignOutStarted(nullptr, args);
    });
}

SampleGame::~SampleGame()
{
    delete StateManager;
}

// Initialize the Direct3D resources required to run.
void SampleGame::Initialize(::IUnknown* window)
{
    m_deviceResources->SetWindow(window);

    PlatformIndependentInitialize(window);
}

void SampleGame::PlatformIndependentInitialize(::IUnknown* window)
{
    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    InputManager = std::make_unique<InputState>();

    InputManager->SetWindow(window);

    LiveResources->Initialize();
}

void SampleGame::CreateDeviceDependentResources()
{
    m_graphicsMemory = std::make_unique<GraphicsMemory>(m_deviceResources->GetD3DDevice(), m_deviceResources->GetBackBufferCount());

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

    // Prepare the render target to render a new frame.
    m_deviceResources->Prepare();

    Clear();

    auto context = m_deviceResources->GetD3DDeviceContext();
    PIXBeginEvent(context, EVT_COLOR_RENDER, L"Render");

    // Render the scene objects
    ScreenManager->Render(m_timer);

    PIXEndEvent(context);

    // Present the contents of the swap chain to the screen
    PIXBeginEvent(context, EVT_COLOR_PRESENT, L"Present");

    m_deviceResources->Present();

    m_graphicsMemory->Commit();

    PIXEndEvent(context);
}

void SampleGame::OnSuspending(winrt::Windows::ApplicationModel::SuspendingDeferral const & deferral)
{
    Log::Write(L"OnSuspending()\n");

    auto context = m_deviceResources->GetD3DDeviceContext();

    context->Suspend(0);

    StateManager->Suspend();
    InputManager->Suspend();
	SuspendAsyncActions(deferral);
}


winrt::Windows::Foundation::IAsyncAction SampleGame::SuspendAsyncActions(winrt::Windows::ApplicationModel::SuspendingDeferral deferral)
{
	co_await GameSaveManager.Suspend();
	Log::WriteAndDisplay(L"OnSuspending() complete\n");
	deferral.Complete();
}

void SampleGame::OnResuming()
{
    Log::Write(L"OnResuming()\n");

    auto context = m_deviceResources->GetD3DDeviceContext();
    context->Resume();

    InputManager->Resume();
    StateManager->Resume();
    m_timer.ResetElapsedTime();
}

void SampleGame::GetDefaultSize(int& width, int& height) const
{
    width = 1920;
    height = 1080;
}

std::future<HRESULT> SampleGame::InitializeGameSaveSystem()
{
    Log::Write(L"SampleGame::InitializeGameSaveSystem()\n");

    if (LiveResources->IsUserSignedIn())
    {
        auto xblUser = LiveResources->GetUser();
        return this->GameSaveManager.InitializeForUser(xblUser, false);
    }
	else
	{
		Log::WriteAndDisplay(L"ERROR initializing game save system: Xbox Live User not signed in\n");
		std::packaged_task<HRESULT ()> task([] { return E_FAIL; });
		return task.get_future();
    }
}

void SampleGame::SetCurrentGamepad(winrt::Windows::Xbox::Input::IGamepad const & gamepad)
{
    m_gamepad = gamepad;
    m_gamepadIndex = (gamepad == nullptr) ? -1 : InputManager->GetGamepadIndex(gamepad);
    if (ScreenManager != nullptr)
    {
        ScreenManager->UpdateControllingPlayer(m_gamepadIndex);
    }
}

bool SampleGame::UpdateCurrentGamepad()
{
    auto xblUser = LiveResources->GetUser();
    if (xblUser == nullptr)
    {
        SetCurrentGamepad(nullptr);
        return false;
    }

    if (m_gamepad != nullptr)
    {
        // is the active user already paired with the current gamepad?
        if (m_gamepad.User() != nullptr && (m_gamepad.User().Id() == xblUser.Id()))
        {
            SetCurrentGamepad(m_gamepad);
            Log::WriteAndDisplay(L"Gamepad refreshed for current user (index = %d)\n", m_gamepadIndex);
            return true;
        }
    }

    // is the active user paired with another gamepad?
    for (const auto& gamepad : winrt::Windows::Xbox::Input::Gamepad::Gamepads())
    {
        if (gamepad.User() != nullptr && gamepad.User().Id() == xblUser.Id())
        {
            SetCurrentGamepad(gamepad);
            Log::WriteAndDisplay(L"New gamepad set for current user (index = %d)\n", m_gamepadIndex);
            return true;
        }
    }

    SetCurrentGamepad(nullptr);
    return false;
}

} // namespace GameSaveSample
