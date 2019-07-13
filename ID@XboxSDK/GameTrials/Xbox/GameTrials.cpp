//--------------------------------------------------------------------------------------
// GameTrials.cpp
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "GameTrials.h"

#include "ATGColors.h"

extern void ExitSample();

using namespace DirectX;

using Microsoft::WRL::ComPtr;

namespace
{
    const int c_sampleUIOverlay = 2000;
    const int c_modeLabel = 2001;
    const int c_ctfButton = 2002;
    const int c_kothButton = 2003;
    const int c_mitmButton = 2004;
    const int c_domButton = 2005;

    const int c_purchasePopUp = 3000;
    const int c_switchUserPopUp = 4000;
    const int c_messagePopUp = 5000;

    const int c_actionBtn = 1;
    const int c_closeBtn = 2;
    const int c_messageLabel = 10;

    const int c_timeRemainingLabel = 2006;
    const int c_playerMessageLabel = 2007;
    const int c_licenseHolderLabel = 2008;

    const wchar_t *c_productId = L"613925c8-e7f9-4ed4-bfc4-9187695284dd";

    std::wstring FormatTime(uint64_t time)
    {
        uint64_t seconds = time % 60;
        uint64_t minutes = (time / 60) % 60;
        uint64_t hours = time / 3600;

        wchar_t buffer[1024] = {};
        swprintf_s(buffer, L"%02lld:%02lld:%02lld", hours, minutes, seconds);

        return std::wstring(buffer);
    }

    std::wstring GetTimeRemainingMessage(uint64_t time)
    {
        uint64_t minutes = (time / 60) % 60;
        uint64_t hours = time / 3600;

        if (hours > 0)
        {
            wchar_t buffer[1024] = {};
            swprintf_s(buffer, L"Less than %lld hours remaining.", hours + 1);
            return buffer;
        }
        else if (minutes > 30)
        {
            return L"Less than 1 hour remaining.";
        }
        else if (minutes > 15)
        {
            return L"Less than 30 minutes remaining.";
        }
        else
        {
            return  L"Your trial will be ending soon.";
        }

    }
}

Sample::Sample() :
    m_frame(0)
{
    // Renders only 2D, so no need for a depth buffer.
    m_deviceResources = std::make_unique<DX::DeviceResources>(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_UNKNOWN);

    m_liveResources = std::make_shared<ATG::LiveResources>();
    m_liveInfoHUD = std::make_unique<ATG::LiveInfoHUD>(L"Game Trials Sample");

    ATG::UIConfig uiconfig;
    m_ui = std::make_unique<ATG::UIManager>(uiconfig);
}

// Initialize the Direct3D resources required to run.
void Sample::Initialize(IUnknown* window)
{
    m_gamePad = std::make_unique<GamePad>();

    m_ui->LoadLayout(L".\\Assets\\SampleUI.csv", L".\\Assets");

    m_deviceResources->SetWindow(window);

    m_deviceResources->CreateDeviceResources();  
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    m_liveResources->SetUserChangedCallback([this](ATG::XboxLiveUser user)
    {
        m_liveInfoHUD->SetUser(m_liveResources->GetLiveContext());
        m_ui->FindPanel<ATG::Overlay>(c_sampleUIOverlay)->Show();
    });

    m_liveResources->SetUserSignOutCompletedCallback([this](ATG::XboxLiveUser user)
    {
        m_liveInfoHUD->SetUser(m_liveResources->GetLiveContext());
        m_ui->FindPanel<ATG::Overlay>(c_sampleUIOverlay)->Close();
    });

    m_liveResources->Initialize();
    m_liveInfoHUD->Initialize(m_liveResources->GetLiveContext());

    // The license can change when users sign in and out.  When this happens we need to refresh
    // what the title sees as the license.
    Windows::ApplicationModel::Store::CurrentApp::LicenseInformation->LicenseChanged +=
        ref new Windows::ApplicationModel::Store::LicenseChangedEventHandler([this]()
        {
             RefreshLocalLicenseInfo();
        });

    RefreshLocalLicenseInfo();

    SetupUI();
}

#pragma region Trial Methods
void Sample::RefreshLocalLicenseInfo()
{
    // Clear out the information we store about the license.
    m_licensedUserGamerTag.clear();
    m_isTrial = false;
    m_remainingTime = 0;

    try
    {
        auto info = Windows::ApplicationModel::Store::CurrentApp::LicenseInformation;

        m_isTrial = info->IsTrial;

        // If we aren't in a trial then we don't need to set up the timers.
        if(m_isTrial)
        {
            m_ui->FindControl<ATG::TextLabel>(c_sampleUIOverlay, c_modeLabel)->SetText(L"Trial Mode");
            
            FILETIME currentSystemTime = {};
            ::GetSystemTimeAsFileTime(&currentSystemTime);
            ULARGE_INTEGER currentTime = { currentSystemTime.dwLowDateTime, currentSystemTime.dwHighDateTime };

            m_remainingTime = currentTime.QuadPart;

            // Make sure that a XUID is attached to the license.
            if(info->CurrentLicenseUserXuid)
            {
                m_licensedUserXUID = info->CurrentLicenseUserXuid->Data();

                // If the signed in user is the same as the licensed user, then we can just use the local
                // user's gamertag.
                if (m_licensedUserXUID == m_liveResources->GetXuid())
                {
                    m_licensedUserGamerTag = m_liveResources->GetGamertag();
                }
                else
                {
                    // Otherwise we need to do to the profile service.
                    m_liveResources->GetLiveContext()->profile_service().get_user_profile(m_licensedUserXUID)
                        .then([&](xbox::services::xbox_live_result<xbox::services::social::xbox_user_profile> result)
                        {
                            if(result.err())
                            {
                                ShowMessagePopUp(L"Unable to retrieve GamerTag of licensed user.");
                                return;
                            }

                            auto &profile = result.payload();

                            m_licensedUserGamerTag = profile.gamertag();
                        });
                }
            }
        }
        else
        {
            m_ui->FindControl<ATG::TextLabel>(c_sampleUIOverlay, c_modeLabel)->SetText(L"Full Game");
            m_licensedUserXUID = m_liveResources->GetXuid();
            m_licensedUserGamerTag = m_liveResources->GetGamertag();

            m_ui->FindControl<ATG::TextLabel>(c_sampleUIOverlay, c_timeRemainingLabel)->SetText(L"");
            m_ui->FindControl<ATG::TextLabel>(c_sampleUIOverlay, c_playerMessageLabel)->SetText(L"");
            m_ui->FindControl<ATG::TextLabel>(c_sampleUIOverlay, c_licenseHolderLabel)->SetText(m_licensedUserGamerTag.c_str());
        }
    }
    catch (Platform::Exception ^ex)
    {
        ShowMessagePopUp(L"Error while retrieving license.");
    }
}

void Sample::UpdateTimeRemaining(uint64_t delta)
{
    m_remainingTime -= delta;
}
#pragma endregion

#pragma region UI Methods
void Sample::SetupUI()
{
    using namespace ATG;

    auto gameModeSelection = [this](IPanel*, IControl* ctrl)
    {
        // Monkey in the Middle and Domination are only available in the full game
        if (ctrl->GetId() > c_kothButton && m_isTrial)
        {
            ShowPurchasePopUp();
        }
        else
        {
            ShowMessagePopUp(L"Game mode unlocked and playable!");
        }
    };

    m_ui->FindControl<IControl>(c_sampleUIOverlay,c_ctfButton)->SetCallback(gameModeSelection);
    m_ui->FindControl<IControl>(c_sampleUIOverlay, c_kothButton)->SetCallback(gameModeSelection);
    m_ui->FindControl<IControl>(c_sampleUIOverlay, c_mitmButton)->SetCallback(gameModeSelection);
    m_ui->FindControl<IControl>(c_sampleUIOverlay, c_domButton)->SetCallback(gameModeSelection);

    m_ui->FindControl<IControl>(c_purchasePopUp, c_actionBtn)->SetCallback([this](IPanel *panel, IControl*)
    {
        // You should use ShowPurchaseAsync in a real trial as it reduces the number steps between the player and purchasing the full game.
        // However, if you have bundles that you would want to try to up sell, then ShowDetailsAsync would be the correct choice.
        Windows::Xbox::ApplicationModel::Store::Product::ShowDetailsAsync(m_liveResources->GetUser(), ref new Platform::String(c_productId));
		panel->Close();
    });

    m_ui->FindControl<IControl>(c_switchUserPopUp, c_actionBtn)->SetCallback([this](IPanel *panel, IControl*)
    {
        Windows::Xbox::UI::SystemUI::ShowAccountPickerAsync(nullptr, Windows::Xbox::UI::AccountPickerOptions::None);
        panel->Close();
    });

}

void Sample::ShowPurchasePopUp()
{
    m_ui->FindPanel<ATG::IPanel>(c_purchasePopUp)->Show();
}

void Sample::ShowChangeUserPopUp()
{
    wchar_t buffer[1024] = {};
    swprintf_s(buffer, L"Trial is licensed to %ls. Please sign in with the trial owner.", m_licensedUserGamerTag.c_str());
    m_ui->FindControl<ATG::TextLabel>(c_switchUserPopUp,c_messageLabel)->SetText(buffer);
    m_ui->FindPanel<ATG::IPanel>(c_switchUserPopUp)->Show();
}

void Sample::ShowMessagePopUp(const wchar_t *message)
{
    m_ui->FindControl<ATG::TextLabel>(c_messagePopUp, c_messageLabel)->SetText(message);
    m_ui->FindPanel<ATG::IPanel>(c_messagePopUp)->Show();
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
    
    if(m_isTrial)
    {
        UpdateTimeRemaining(timer.GetElapsedTicks());

        // Time left on the trial for demonstration.  This is not something that you would directly show the user.
        m_ui->FindControl<ATG::TextLabel>(c_sampleUIOverlay, c_timeRemainingLabel)->SetText(FormatTime(m_remainingTime).c_str());
        // An example of a time remaining message to display to the user.
        m_ui->FindControl<ATG::TextLabel>(c_sampleUIOverlay, c_playerMessageLabel)->SetText(GetTimeRemainingMessage(m_remainingTime).c_str());
        // For debugging purposes this shows the gamertag of the user that holds the trial license.
        m_ui->FindControl<ATG::TextLabel>(c_sampleUIOverlay, c_licenseHolderLabel)->SetText(m_licensedUserGamerTag.c_str());

        // If the current user doesn't match the license holder, then we need to inform the user they need to switch users.  This is 
        // to prevent someone from creating multiple accounts and playing the trial indefinitely.
        if (m_licensedUserXUID != m_liveResources->GetXuid() && !m_ui->FindPanel<ATG::IPanel>(c_switchUserPopUp)->IsVisible())
        {
            ShowChangeUserPopUp();
        }
    }

    elapsedTime;

    auto pad = m_gamePad->GetState(0);
    if (pad.IsConnected())
    {
        m_gamePadButtons.Update(pad);

        if (!m_ui->Update(elapsedTime, pad))
        {
            if (pad.IsViewPressed())
            {
                ExitSample();
            }
            if (m_gamePadButtons.menu == GamePad::ButtonStateTracker::PRESSED)
            {
                Windows::Xbox::UI::SystemUI::ShowAccountPickerAsync(nullptr, Windows::Xbox::UI::AccountPickerOptions::None);
            }
            if (m_gamePadButtons.x == GamePad::ButtonStateTracker::PRESSED)
            {
                RefreshLocalLicenseInfo();
            }
            if (m_gamePadButtons.y == GamePad::ButtonStateTracker::PRESSED)
            {
				Windows::Xbox::ApplicationModel::Store::Product::ShowDetailsAsync(m_liveResources->GetUser(), ref new Platform::String(c_productId));
			}
        }
    }
    else
    {
        m_gamePadButtons.Reset();
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
        
    m_liveInfoHUD->Render();

    m_ui->Render();

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
    auto renderTarget = m_deviceResources->GetRenderTargetView();

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
    m_liveResources->Refresh();
    m_ui->Reset();
    RefreshLocalLicenseInfo();
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Sample::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();

    m_graphicsMemory = std::make_unique<GraphicsMemory>(device, m_deviceResources->GetBackBufferCount());

    m_liveInfoHUD->RestoreDevice(context);
    m_ui->RestoreDevice(context);
}

// Allocate all memory resources that change on a window SizeChanged event.
void Sample::CreateWindowSizeDependentResources()
{
    auto fullscreen = m_deviceResources->GetOutputSize();
    m_ui->SetWindow(fullscreen);
}
#pragma endregion
