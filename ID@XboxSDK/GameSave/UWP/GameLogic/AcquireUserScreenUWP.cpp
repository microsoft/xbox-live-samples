// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "pch.h"
#include "AcquireUserScreen.h"

#include "InputState.h"
#include "LiveResourcesUWP.h"
#include "SampleGame.h"
#include "StateManager.h"

using namespace Concurrency;
using namespace DirectX;
using namespace xbox::services;
using namespace xbox::services::system;

namespace GameSaveSample {

AcquireUserScreen::AcquireUserScreen(const std::shared_ptr<ScreenManager>& screenManager, bool autoSignIn) : MenuScreen(screenManager),
    m_status(AcquireUserStatus::SigningIn)
{
    m_showCurrentUser = false;
    m_transitionOnTime = 1.0f;
    m_transitionOffTime = 1.0f;

    m_dispatcher = Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher;

    if (!Game->LiveResources->IsUserSignedIn())
    {
        if (Windows::System::Profile::AnalyticsInfo::VersionInfo->DeviceFamily == "Windows.Xbox")
        {
            UpdateMenu(AcquireUserStatus::NeedsUserInteraction);
        }
        else
        {
            if (autoSignIn)
            {
                SignIn(SigninMethod::Silent);
            }
            else
            {
                UpdateMenu(AcquireUserStatus::NeedsUserInteraction);
            }
        }
    }
    else
    {
        UpdateMenu(AcquireUserStatus::Ready);
    }
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

void AcquireUserScreen::Update(float totalTime, float elapsedTime, bool otherScreenHasFocus, bool coveredByOtherScreen)
{
    if (m_status == AcquireUserStatus::Ready)
    {
        if (!Game->LiveResources->IsUserSignedIn())
        {
            UpdateMenu(AcquireUserStatus::NeedsUserInteraction);
        }
    }

    MenuScreen::Update(totalTime, elapsedTime, otherScreenHasFocus, coveredByOtherScreen);
}

void AcquireUserScreen::Draw(float totalTime, float elapsedTime)
{
    auto spriteBatch = Manager()->GetSpriteBatch();
    auto blendStates = Manager()->GetCommonStates();
    auto viewportBounds = Manager()->GetScreenBounds();
    float viewportWidth = float(viewportBounds.right);
    float viewportHeight = float(viewportBounds.bottom);
    auto scaleMatrix = DX::GetScaleMatrixForWindow(Manager()->GetWindowBounds());

    spriteBatch->Begin(SpriteSortMode_Deferred, blendStates->NonPremultiplied(), nullptr, nullptr, nullptr, nullptr, scaleMatrix);

    // Draw the background
    RECT backgroundRectangle = { 0, 0, long(viewportWidth), long(viewportHeight) };
    spriteBatch->Draw(m_backgroundTexture->GetResourceViewTemporary(), backgroundRectangle, c_menuBackgroundColor);

    // Draw title logo
    XMFLOAT2 titleLogoPosition = XMFLOAT2(viewportWidth / 2.0f, (viewportHeight / 2.0f) - 200.f);
    XMFLOAT2 titleLogoOrigin = XMFLOAT2(m_titleLogo->Width() / 2.0f, m_titleLogo->Height() / 2.0f);
    spriteBatch->Draw(m_titleLogo->GetResourceViewTemporary(), titleLogoPosition, nullptr, Colors::White, 0.0f, titleLogoOrigin);

    spriteBatch->End();

    critical_section::scoped_lock lock(m_menuLock);
    MenuScreen::Draw(totalTime, elapsedTime);
}

void AcquireUserScreen::OnCancel()
{
    // nothing to do... there's no backing out from here
}

task<void> AcquireUserScreen::AcquireUser(int dxGamepadIndex)
{
    Log::Write("AcquireUserScreen::AcquireUser(%d)\n", dxGamepadIndex);

    Game->LiveResources->SetWindowsUser(nullptr);

    // try to get Windows User from gamepad
    if (dxGamepadIndex != -1)
    {
        auto dxGamepad = Game->InputManager->m_gamePad.get();
        auto gamepadCaps = dxGamepad->GetCapabilities(dxGamepadIndex);

        // match the DirectXTK gamepad to the Windows SDK gamepad to find the Windows user
        for (const auto& gamepad : Windows::Gaming::Input::Gamepad::Gamepads)
        {
            if (gamepad->User && (wcscmp(gamepad->User->NonRoamableId->Data(), gamepadCaps.id.c_str()) == 0))
            {
                Game->LiveResources->SetWindowsUser(gamepad->User);
                return create_task([] {});
            }
        }
    }

    // if User not available from a gamepad, prompt user to select an account
    auto picker = ref new Windows::System::UserPicker();
    picker->AllowGuestAccounts = true;
    return create_task(picker->PickSingleUserAsync()).then([](task<Windows::System::User^> tUserPick)
    {
        try
        {
            auto userPick = tUserPick.get();
            if (userPick)
            {
                Game->LiveResources->SetWindowsUser(userPick);
            }
        }
        catch (Platform::Exception^ ex)
        {
            Log::Write("WARNING: Exception caught from PickSingleUserAsync: %ws (%ws)\n", ex->Message->Data(), FormatHResult(ex->HResult)->Data());
        }
    });
}

void AcquireUserScreen::SignIn(SigninMethod method)
{
    UpdateMenu(AcquireUserStatus::SigningIn);

    auto handleSignInResult = [this](xbox_live_result<sign_in_result>& signInResult)
    {
        if (signInResult.err())
        {
            Game->StateManager->PopupError("Signin Failure: " + ref new Platform::String(utility::conversions::utf8_to_utf16(signInResult.err_message()).c_str()));
        }
        else
        {
            switch (signInResult.payload().status())
            {
            case sign_in_status::success:
                Log::WriteAndDisplay("User signed in (%ws)\n", FormatUserName(Game->LiveResources->GetUser())->Data());
                UpdateMenu(AcquireUserStatus::Ready);
                return;
            case sign_in_status::user_cancel:
                Log::Write("User canceled signin\n");
                break;
            case sign_in_status::user_interaction_required:
                Log::Write("User interaction required to sign in\n");
                break;
            }
        }

        UpdateMenu(AcquireUserStatus::NeedsUserInteraction);
    };

    switch (method)
    {
    case SigninMethod::Silent:
        Log::Write("AcquireUserScreen::SignIn(silently)\n");
        Game->LiveResources->SignInSilently(handleSignInResult, m_dispatcher);
        break;
    case SigninMethod::Normal:
        Log::Write("AcquireUserScreen::SignIn(with UI)\n");
        Game->LiveResources->SignIn(handleSignInResult, m_dispatcher);
        break;
    }
}

void AcquireUserScreen::UpdateMenu(AcquireUserStatus status)
{
    critical_section::scoped_lock lock(m_menuLock);
    m_status = status;
    ClearMenuEntries();

    switch (status)
    {
    case AcquireUserStatus::SigningIn:
        m_menuEntries.push_back(MenuEntry(L"Signing in..."));
        break;

    case AcquireUserStatus::NeedsUserInteraction:
        m_menuEntries.push_back(MenuEntry(L"Select Profile",
            [this](int selectingGamepad)
        {
            Log::Write("Select Profile::Gamepad %d\n", selectingGamepad);
            if (Windows::System::Profile::AnalyticsInfo::VersionInfo->DeviceFamily == "Windows.Xbox")
            {
                AcquireUser(selectingGamepad).then([this]
                {
                    if (Game->LiveResources->GetWindowsUser())
                    {
                        SignIn(SigninMethod::Normal);
                    }
                    else
                    {
                        Game->StateManager->PopupError("Signin Failure: Could not acquire user");
                    }
                });
            }
            else
            {
                SignIn(SigninMethod::Normal);
            }
        }));
        break;

    case AcquireUserStatus::Ready:
        {
            std::wstring startMenuEntry = L"Continue as ";
            startMenuEntry += Game->LiveResources->GetGamertag();
            m_menuEntries.push_back(MenuEntry(startMenuEntry,
                [this](int selectingGamepad)
            {
                Log::Write("Continue::Gamepad %d\n", selectingGamepad);
                UpdateMenu(AcquireUserStatus::Exiting);
                Game->StateManager->SwitchState(GameState::InitializeGameSaveSystem);
            }));
        }

        if (Windows::System::Profile::AnalyticsInfo::VersionInfo->DeviceFamily == "Windows.Xbox")
        {
            m_menuEntries.push_back(MenuEntry(L"Select Profile",
                [this](int)
            {
                AcquireUser(-1).then([this]
                {
                    SignIn(SigninMethod::Normal);
                });
            }));
        }
        break;

    case AcquireUserStatus::Exiting:
        m_menuEntries.push_back(MenuEntry(L"Loading . . ."));
        break;
    }
}

} // namespace GameSaveSample
