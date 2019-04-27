//--------------------------------------------------------------------------------------
// AcquireUserScreenXDK.cpp
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "AcquireUserScreen.h"

#include "InputState.h"
#include "SampleGame.h"
#include "StateManager.h"

using namespace Concurrency;
using namespace DirectX;
using namespace winrt::Windows::Xbox::Input;
using namespace winrt::Windows::Xbox::UI;
using namespace winrt::Windows::Xbox::System;

namespace GameSaveSample {

AcquireUserScreen::AcquireUserScreen(const std::shared_ptr<ScreenManager>& screenManager, bool autoSignIn) : MenuScreen(screenManager),
    m_status(AcquireUserStatus::SigningIn)
{
    UNREFERENCED_PARAMETER(autoSignIn);

    m_transitionOnTime = 1.0f;
    m_transitionOffTime = 0.0f;

    m_menuEntries.push_back(MenuEntry(L"Continue",
        [this](int selectingPlayer) -> winrt::Windows::Foundation::IAsyncAction
    {
        AcquireUser(selectingPlayer);
		return winrt::Windows::Foundation::IAsyncAction(nullptr);
    }));

    m_menuEntries.push_back(MenuEntry(L"Select Profile",
        [this](int selectingPlayer) -> winrt::Windows::Foundation::IAsyncAction
    {
        SwitchUser(selectingPlayer);
		return winrt::Windows::Foundation::IAsyncAction(nullptr);
    }));
}

AcquireUserScreen::~AcquireUserScreen()
{
}

void AcquireUserScreen::LoadContent()
{
    MenuScreen::LoadContent();

    m_backgroundTexture = Manager()->GetContentManager()->LoadTexture(L"Assets\\blank.png");
    m_titleLogo = Manager()->GetContentManager()->LoadTexture(L"Assets\\WordGame_logo.png");
}

void AcquireUserScreen::Draw(float totalTime, float elapsedTime)
{
    auto spriteBatch = Manager()->GetSpriteBatch();
    auto blendStates = Manager()->GetCommonStates();
    auto viewportBounds = Manager()->GetScreenBounds();
    float viewportWidth = float(viewportBounds.right);
    float viewportHeight = float(viewportBounds.bottom);

    spriteBatch->Begin(SpriteSortMode_Deferred, blendStates->NonPremultiplied());

    // Draw the background
    RECT backgroundRectangle = { 0, 0, long(viewportWidth), long(viewportHeight) };
    spriteBatch->Draw(m_backgroundTexture->GetResourceViewTemporary(), backgroundRectangle, c_menuBackgroundColor);

    // Draw title logo
    XMFLOAT2 titleLogoPosition = XMFLOAT2(viewportWidth / 2.0f, (viewportHeight / 2.0f) - 200.f);
    XMFLOAT2 titleLogoOrigin = XMFLOAT2(m_titleLogo->Width() / 2.0f, m_titleLogo->Height() / 2.0f);
    spriteBatch->Draw(m_titleLogo->GetResourceViewTemporary(), titleLogoPosition, nullptr, Colors::White, 0.0f, titleLogoOrigin);

    spriteBatch->End();

    MenuScreen::Draw(totalTime, elapsedTime);
}

void AcquireUserScreen::HandleInput(const DirectX::InputState& input)
{
    if (m_status == AcquireUserStatus::Exiting)
    {
        return;
    }

    MenuScreen::HandleInput(input);
}

void AcquireUserScreen::OnCancel()
{
    // nothing to do... there's no backing out from here
}

void AcquireUserScreen::AcquireUser(int dxGamepadIndex)
{
    Log::Write(L"AcquireUserScreen::AcquireUser(%d)\n", dxGamepadIndex);

    auto gamepad = Game->InputManager->GetGamepad(dxGamepadIndex);
    if (gamepad == nullptr)
    {
        Game->StateManager->PopupError(L"No gamepad found at index " + dxGamepadIndex);
        return;
    }

    User gamepadUser = gamepad.User();
    if (gamepadUser == nullptr)
    {
        SelectUser(gamepad);
    }
    else
    {
        // Set the current user info and advance state
        PrepareToExit(gamepad);
    }
}

winrt::Windows::Foundation::IAsyncAction AcquireUserScreen::SelectUser(IGamepad gamepad)
{
	auto result = co_await SystemUI::ShowAccountPickerAsync(gamepad, winrt::Windows::Xbox::UI::AccountPickerOptions::None);

    {
        if (result.User() == nullptr)
        {
            // User canceled the dialog
            co_return;
        }

        // Make sure everything is bound correctly
        if (gamepad.User() == nullptr ||
            wcscmp(result.User().XboxUserId().c_str(), gamepad.User().XboxUserId().c_str()) != 0)
        {
            Game->StateManager->PopupError(L"Could not pair controller to user");
            co_return;
        }

        // Set the current user info and advance state
        PrepareToExit(gamepad);
    };
}

void AcquireUserScreen::SwitchUser(int dxGamepadIndex)
{
    Log::Write(L"AcquireUserScreen::SwitchUser(%d)\n", dxGamepadIndex);

    auto gamepad = Game->InputManager->GetGamepad(dxGamepadIndex);
    if (gamepad == nullptr)
    {
        Game->StateManager->PopupError(L"No gamepad found at index " + dxGamepadIndex);
        return;
    }

    SelectUser(gamepad);
}

void AcquireUserScreen::PrepareToExit(winrt::Windows::Xbox::Input::IGamepad const & gamepad)
{
    assert(gamepad.User() != nullptr);

    // Make sure LiveResources is using the same User
    Game->LiveResources->SetCurrentUser(gamepad.User(), false);
    Log::WriteAndDisplay(L"User set (%ws)\n", FormatUserName(gamepad.User()).c_str());

    Game->SetCurrentGamepad(gamepad);
    Log::WriteAndDisplay(L"Set gamepad for current user (index = %d)\n", Game->GetCurrentGamepadIndex());

    ClearMenuEntries();
    m_menuEntries.push_back(MenuEntry(L"Loading . . ."));
    m_status = AcquireUserStatus::Exiting;

    Game->StateManager->SwitchState(GameState::InitializeGameSaveSystem);
}

} // namespace GameSaveSample
