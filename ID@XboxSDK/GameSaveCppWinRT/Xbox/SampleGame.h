//--------------------------------------------------------------------------------------
// SampleGame.h
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include "DeviceResources.h"
#include "GameSaveManager.h"
#include "InputState.h"
#include "LiveResourcesCppWinRT_XDK.h"
#include "ScreenManager.h"
#include "StateManager.h"
#include "StepTimer.h"

namespace GameSaveSample
{
    // PIX event colors
    const UINT EVT_COLOR_UPDATE  = PIX_COLOR_INDEX(1);
    const UINT EVT_COLOR_CLEAR   = PIX_COLOR_INDEX(2);
    const UINT EVT_COLOR_RENDER  = PIX_COLOR_INDEX(3);
    const UINT EVT_COLOR_PRESENT = PIX_COLOR_INDEX(4);

    // Creates a D3D11 device and provides a render loop.
    class SampleGame
	{
	public:
        SampleGame();
        ~SampleGame();

        // Initialization and device management
        void Initialize(::IUnknown* window);

    private:
        void PlatformIndependentInitialize(::IUnknown* window);
        void CreateDeviceDependentResources();
        void CreateWindowSizeDependentResources();

    public:
        // Basic game loop
        void Tick();
        void Update(DX::StepTimer const& timer);
        void Clear();
        void Render();

#ifndef _XBOX_ONE
        // IDeviceNotify
        virtual void OnDeviceLost() override;
        virtual void OnDeviceRestored() override;
#endif

        // Event handlers
		void OnSuspending(winrt::Windows::ApplicationModel::SuspendingDeferral const & deferral);
		winrt::Windows::Foundation::IAsyncAction SuspendAsyncActions(winrt::Windows::ApplicationModel::SuspendingDeferral deferral);
        void OnResuming();
#ifndef _XBOX_ONE
        void OnKeyUp(Windows::UI::Core::KeyEventArgs^ args);
        void OnWindowActivated();
        void OnWindowDeactivated();
        void OnWindowSizeChanged(int width, int height, DXGI_MODE_ROTATION rotation, float dpi);
        void ValidateDevice();
#endif

        // Properties
        void GetDefaultSize(int& width, int& height) const;

        // Game save functions
        std::future<HRESULT> InitializeGameSaveSystem();

        // Game Save Manager
        GameSaveManager GameSaveManager;

        // Input Manager
        std::unique_ptr<DirectX::InputState> InputManager;

        // Screen Manager (updates and renders all game screens)
        std::shared_ptr<ScreenManager> ScreenManager;

        // State Manager
        StateManager* StateManager;

		// Xbox Live User Management
		std::shared_ptr<ATG::LiveResources> LiveResources;
        
        /////////////////////////////////////////////////////////////////
        //
        // Gamepad and Gamepad Index
        //

        int GetCurrentGamepadIndex() { return m_gamepadIndex; }

        // Returns current gamepad paired to current user
        winrt::Windows::Xbox::Input::IGamepad GetCurrentGamepad() { return m_gamepad; }

        // Sets current gamepad and gamepad index
        void SetCurrentGamepad(winrt::Windows::Xbox::Input::IGamepad const & gamepad);

        // Returns true if a gamepad was found for Current User, or false if a gamepad could not be found for the Current User
        bool UpdateCurrentGamepad();

    private:
        std::shared_ptr<DX::DeviceResources>                m_deviceResources;
        winrt::Windows::Xbox::Input::IGamepad               m_gamepad;
        int                                                 m_gamepadIndex;
        std::unique_ptr<DirectX::GraphicsMemory>            m_graphicsMemory;
        DX::StepTimer                                       m_timer;
    };
}

extern GameSaveSample::SampleGame* g_gameInstance;
