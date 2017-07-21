// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "pch.h"
#include "SimplifiedAchievements.h"
#include "ATGColors.h"

using namespace DirectX;

namespace
{
    const wchar_t *ProgressStateText[] = {
        L"Unknown",
        L"Achieved",
        L"Not Started",
        L"In Progress"
    };

    const wchar_t *AchievementTypeText[] = {
        L"Unknown",
        L"All",
        L"Persistent",
        L"Challenge"
    };

    const int c_maxAchievements          = 10;
    const int c_liveHUD                  = 1000;
    const int c_sampleUIPanel            = 2000;
    const int c_getAchievementsBtn       = 2101;
    const int c_getSingleAchievementBtn  = 2102;
    const int c_updateAchievementBtn     = 2104;

    const string_t c_startedTheGameAchievementId = L"1";
    const string_t c_killedEnemyAchievementId = L"2";
}

Sample::Sample() :
    m_frame(0)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_UNKNOWN);
    m_liveResources = std::make_unique<ATG::LiveResources>();

    ATG::UIConfig uiconfig;
    m_ui = std::make_unique<ATG::UIManager>(uiconfig);

}

// Initialize the Direct3D resources required to run.
void Sample::Initialize(IUnknown* window)
{
    m_gamePad = std::make_unique<GamePad>();

    m_ui->LoadLayout(L".\\Assets\\SampleUI.csv", L".\\Assets");

    m_liveResources->Initialize(m_ui, m_ui->FindPanel<ATG::Overlay>(c_sampleUIPanel));
    m_deviceResources->SetWindow(window);

    m_deviceResources->CreateDeviceResources();  
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();
    
    SetupUI();

    if (m_liveResources != nullptr && m_liveResources->GetLiveContext() != nullptr)
    {
        UpdateAchievement(c_startedTheGameAchievementId);
    }
}

#pragma region Achievement Methods

void Sample::GetAllAchievements()
{
    auto& achievementService = m_liveResources->GetLiveContext()->achievement_service();
    achievementService.get_achievements_for_title_id(
        m_liveResources->GetUser()->XboxUserId->Data(),                     // Xbox LIVE user Id
        m_liveResources->GetTitleId(),                                      // Title Id to get achievement data for
        xbox::services::achievements::achievement_type::all,                // achievement_type filter: All mean to get Persistent and Challenge achievements
        false,                                                              // All possible achievements including accurate unlocked data
        xbox::services::achievements::achievement_order_by::title_id,       // achievement_order_by filter: Default means no particular order
        0,                                                                  // The number of achievement items to skip
        c_maxAchievements                                                   // The maximum number of achievement items to return in the response
        )
    .then([this](xbox::services::xbox_live_result<xbox::services::achievements::achievements_result> result)
    {
        this->ProcessAchievments(result);
    });
}

void Sample::GetAchievement()
{
    auto& achievementService = m_liveResources->GetLiveContext()->achievement_service();
    achievementService.get_achievement(
        m_liveResources->GetUser()->XboxUserId->Data(),          // Xbox LIVE user Id
        m_liveResources->GetServiceConfigId(),                   // Achievements are associated with a service configuration Id
        c_startedTheGameAchievementId                            // The achievement Id of the achievement being requested
        )
    .then([this](xbox::services::xbox_live_result<xbox::services::achievements::achievement> result)
    {
        if (!result.err())
        {
            this->PrintAchievement(result.payload());
        }
    });
}

void Sample::UpdateAchievement(_In_ const string_t& achievementId)
{
    if (m_liveResources == nullptr)
        return;
    if (m_liveResources->GetLiveContext() == nullptr)
        return;

    auto& achievementService = m_liveResources->GetLiveContext()->achievement_service();
    achievementService.update_achievement(
        m_liveResources->GetUser()->XboxUserId->Data(),
        achievementId,
        100
    )
    .then([this](xbox::services::xbox_live_result<void> result)
    {
        if (!result.err())
        {
            m_console->Format(L"Achievement unlocked\n");
        }
    });
}

void Sample::ProcessAchievments(xbox::services::xbox_live_result<xbox::services::achievements::achievements_result> result)
{
    if (!result.err())
    {
        auto payload = result.payload();

        for (auto &achievement : payload.items())
        {
            PrintAchievement(achievement);
        }

        // Keep processing if there are more Achievements.
        if (payload.has_next())
        {
            payload.get_next(c_maxAchievements).then(
                [this](xbox::services::xbox_live_result<xbox::services::achievements::achievements_result> result)
            {
                this->ProcessAchievments(result);
            });
        }
    }
}

void Sample::PrintAchievement(const xbox::services::achievements::achievement &ach)
{
    m_console->Format(L"Achievement Id: %ls\n", ach.id().c_str());
    m_console->Format(L"Name: %ls\n", ach.name().c_str());
    if (ach.progress_state() == xbox::services::achievements::achievement_progress_state::achieved)
    {
        m_console->Format(L"Description: %ls\n", ach.unlocked_description().c_str());
    }
    else
    {
        m_console->Format(L"Description: %ls\n", ach.locked_description().c_str());
    }
    m_console->Format(L"Achievement Type: %ls\n", AchievementTypeText[(int)ach.type()]);
    m_console->Format(L"Progress State: %ls\n", ProgressStateText[(int)ach.progress_state()]);
}

#pragma endregion

#pragma region UI Methods
void Sample::SetupUI()
{
    using namespace ATG;

    // Get all Achievements
    m_ui->FindControl<Button>(c_sampleUIPanel, c_getAchievementsBtn)->SetCallback([this](IPanel*, IControl*)
    {
        m_console->Clear();
        GetAllAchievements();
    });

    // Get single Achievement
    m_ui->FindControl<Button>(c_sampleUIPanel, c_getSingleAchievementBtn)->SetCallback([this](IPanel*, IControl*)
    {
        m_console->Clear();
        GetAchievement();
    });

    // Update Achievement
    m_ui->FindControl<Button>(c_sampleUIPanel, c_updateAchievementBtn)->SetCallback([this](IPanel*, IControl*)
    {
        m_console->Clear();
        UpdateAchievement(c_killedEnemyAchievementId);
    });
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

    auto pad = m_gamePad->GetState(0);
    if (pad.IsConnected())
    {
        m_gamePadButtons.Update(pad);

        if (!m_ui->Update(elapsedTime, pad))
        {
            if (pad.IsViewPressed())
            {
                Windows::ApplicationModel::Core::CoreApplication::Exit();
            }
            if (pad.IsMenuPressed())
            {
                Windows::Xbox::UI::SystemUI::ShowAccountPickerAsync(nullptr,Windows::Xbox::UI::AccountPickerOptions::None);
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

    PIXEndEvent(context);

    m_ui->Render();
    m_console->Render();
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
    m_ui->Reset();
    m_liveResources->Refresh();
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Sample::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();

    m_graphicsMemory = std::make_unique<GraphicsMemory>(device, m_deviceResources->GetBackBufferCount());
    m_console = std::make_unique<DX::TextConsole>(context,L"Courier_16.spritefont");

    m_ui->RestoreDevice(context);
}

// Allocate all memory resources that change on a window SizeChanged event.
void Sample::CreateWindowSizeDependentResources()
{
    RECT fullscreen = m_deviceResources->GetOutputSize();
    static const RECT consoleDisplay = { 960, 150, 1838, 825 };

    m_ui->SetWindow(fullscreen);
    m_console->SetWindow(consoleDisplay);
}
#pragma endregion
