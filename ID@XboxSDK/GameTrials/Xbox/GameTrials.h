//--------------------------------------------------------------------------------------
// GameTrials.h
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "SampleGUI.h"
#include "LiveResourcesXDK.h"
#include "LiveInfoHUD.h"


// A basic sample implementation that creates a D3D11 device and
// provides a render loop.
class Sample
{
public:

    Sample();

    // Initialization and management
    void Initialize(IUnknown* window);

    // Basic game loop
    void Tick();

    // Messages
    void OnSuspending();
    void OnResuming();

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    void SetupUI();
    void ShowPurchasePopUp();
    void ShowChangeUserPopUp();
    void ShowMessagePopUp(const wchar_t *message);

    void UpdateTimeRemaining(uint64_t delta);
    void RefreshLocalLicenseInfo();

    // Device resources.
    std::unique_ptr<DX::DeviceResources>        m_deviceResources;

    // Rendering loop timer.
    uint64_t                                    m_frame;
    DX::StepTimer                               m_timer;

    // Input devices.
    std::unique_ptr<DirectX::GamePad>           m_gamePad;

    DirectX::GamePad::ButtonStateTracker        m_gamePadButtons;

    // DirectXTK objects.
    std::unique_ptr<DirectX::GraphicsMemory>    m_graphicsMemory;
	
	// Xbox Live objects.
	std::shared_ptr<ATG::LiveResources>         m_liveResources;
	std::unique_ptr<ATG::LiveInfoHUD>           m_liveInfoHUD;

    // UI Objects
    std::unique_ptr<ATG::UIManager>             m_ui;

    // License Info
    bool                                        m_isTrial;
    uint64_t                                    m_remainingTime;
    std::wstring                                m_licensedUserXUID;
    std::wstring                                m_licensedUserGamerTag;
};
