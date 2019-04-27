// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "pch.h"
#include "AcquireUserScreen.h"

#include "InputState.h"
#include "SampleGame.h"
#include "StateManager.h"

using namespace Concurrency;
using namespace DirectX;
using namespace Windows::Xbox::Input;
using namespace Windows::Xbox::UI;
using namespace Windows::Xbox::System;

namespace GameSaveSample {

AcquireUserScreen::AcquireUserScreen(const std::shared_ptr<ScreenManager>& screenManager, bool autoSignIn) : MenuScreen(screenManager),
    m_status(AcquireUserStatus::SigningIn)
{
    UNREFERENCED_PARAMETER(autoSignIn);

    m_transitionOnTime = 1.0f;
    m_transitionOffTime = 0.0f;

    m_menuEntries.push_back(MenuEntry(L"Continue",
        [this](int selectingPlayer)
    {
        AcquireUser(selectingPlayer);
    }));

    m_menuEntries.push_back(MenuEntry(L"Select Profile",
        [this](int selectingPlayer)
    {
        SwitchUser(selectingPlayer);
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
    Log::Write("AcquireUserScreen::AcquireUser(%d)\n", dxGamepadIndex);

    auto gamepad = Game->InputManager->GetGamepad(dxGamepadIndex);
    if (gamepad == nullptr)
    {
        Game->StateManager->PopupError("No gamepad found at index " + dxGamepadIndex);
        return;
    }

    User^ gamepadUser = gamepad->User;
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

void AcquireUserScreen::SelectUser(IGamepad^ gamepad)
{
    create_task(SystemUI::ShowAccountPickerAsync(gamepad, Windows::Xbox::UI::AccountPickerOptions::None)).then
        ([this, gamepad](task<AccountPickerResult^> t)
    {
        try
        {
            AccountPickerResult^ result = t.get();

            if (result->User == nullptr)
            {
                // User canceled the dialog
                return;
            }

            // Make sure everything is bound correctly
            if (gamepad->User == nullptr ||
                !result->User->XboxUserId->Equals(gamepad->User->XboxUserId))
            {
                Game->StateManager->PopupError("Could not pair controller to user");
                return;
            }

            // Set the current user info and advance state
            PrepareToExit(gamepad);
        }
        catch (Platform::Exception^ ex)
        {
            Platform::String^ errorMsg = "Could not select a user. ShowAccountPickerAsync threw error: " + GetErrorStringForException(ex) + "\n";
            Game->StateManager->PopupError(errorMsg);
        }
        catch (concurrency::task_canceled&)
        {
            Log::Write("ShowAccountPickerAsync canceled\n");
        }
    });
}

void AcquireUserScreen::SwitchUser(int dxGamepadIndex)
{
    Log::Write("AcquireUserScreen::SwitchUser(%d)\n", dxGamepadIndex);

    auto gamepad = Game->InputManager->GetGamepad(dxGamepadIndex);
    if (gamepad == nullptr)
    {
        Game->StateManager->PopupError("No gamepad found at index " + dxGamepadIndex);
        return;
    }

    SelectUser(gamepad);
}

void AcquireUserScreen::PrepareToExit(Windows::Xbox::Input::IGamepad^ gamepad)
{
    assert(gamepad->User != nullptr);

    // Make sure LiveResources is using the same User
    Game->LiveResources->SetCurrentUser(gamepad->User, false);
    Log::WriteAndDisplay("User set (%ws)\n", FormatUserName(gamepad->User)->Data());

    Game->SetCurrentGamepad(gamepad);
    Log::WriteAndDisplay("Set gamepad for current user (index = %d)\n", Game->GetCurrentGamepadIndex());

    ClearMenuEntries();
    m_menuEntries.push_back(MenuEntry(L"Loading . . ."));
    m_status = AcquireUserStatus::Exiting;

    Game->StateManager->SwitchState(GameState::InitializeGameSaveSystem);
}

} // namespace GameSaveSample
