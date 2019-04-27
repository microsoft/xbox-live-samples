// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "pch.h"
#include "StateManager.h"

#include "AcquireUserScreen.h"
#include "ConfirmPopUpScreen.h"
#include "ErrorPopUpScreen.h"
#include "GameBoardScreen.h"
#include "LaunchOptionsScreen.h"
#include "SampleGame.h"
#include "ScreenManager.h"

using namespace Concurrency;
using namespace Windows::Foundation;

#ifdef _XBOX_ONE
using namespace Windows::Xbox::Input;
using namespace Windows::Xbox::System;
#else
using namespace xbox::services;
using namespace xbox::services::system;
#endif

namespace
{
    static bool s_autoSignIn = true;
}

namespace GameSaveSample {

Platform::String^ StateToString(GameState state)
{
    switch (state)
    {
    case GameState::Reset:
        return "Reset";
    case GameState::Initialize:
        return "Initialize";
    case GameState::AcquireUser:
        return "AcquireUser";
    case GameState::InitializeGameSaveSystem:
        return "InitializeGameSaveSystem";
    case GameState::InGame:
        return "InGame";
    case GameState::Suspended:
        return "Suspended";
    case GameState::Resume:
        return "Resume";
    default:
        break;
    }
    return "(Unhandled State)";
}

StateManager::StateManager(const std::shared_ptr<ScreenManager>& screenManager) :
    m_screenManager(screenManager),
    m_priorState(GameState::Reset),
    m_state(GameState::Reset),
    m_pendingState(GameState::Initialize)
{}

///////////////////////////////////////////////////////////////////////////////
//
//  PopupError
//
void StateManager::PopupError(Platform::String^ errorMsg, bool revertToPriorState)
{
    Log::WriteAndDisplay("PopupError: " + errorMsg + "\n");
    m_screenManager->AddScreen(std::make_shared<ErrorPopUpScreen>(m_screenManager, errorMsg), -1);

    if (revertToPriorState)
    {
        RevertToPriorState();
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//  IsCurrentOrPendingState
//      Return TRUE if the current state OR the pending state is equal to the supplied parameter
//
bool StateManager::IsCurrentOrPendingState(GameState state)
{
    critical_section::scoped_lock lock(m_lock);
    return (m_state == state || m_pendingState == state);
}

///////////////////////////////////////////////////////////////////////////////
//
//  RevertToPriorState
//      Circumvents the normal SwitchState functionality - normally used to go back after an error or upon resuming
//
void StateManager::RevertToPriorState()
{
    critical_section::scoped_lock lock(m_lock);
    Log::Write("RevertToPriorState %ws -> %ws\n", StateToString(m_state)->Data(), StateToString(m_priorState)->Data());
    m_pendingState = m_priorState;
    m_priorState = m_state;
    m_state = m_pendingState;
}

///////////////////////////////////////////////////////////////////////////////
//
//  SwitchState
//
void StateManager::SwitchState(GameState newState)
{
    critical_section::scoped_lock lock(m_lock);

    if (newState == GameState::Suspended)
    {
        // Process this state change immediately
        Log::Write("SwitchState %ws -> %ws\n", StateToString(m_state)->Data(), StateToString(newState)->Data());
        m_priorState = m_state;
        m_state = GameState::Suspended;
        m_pendingState = GameState::Suspended;
        return;
    }

    if (m_state == newState)
    {
        Log::Write("WARNING: Attempted to switch to current state.\n");
        return;
    }

    // Set the pending state so it will pick up on the next update tick
    m_pendingState = newState;
}

///////////////////////////////////////////////////////////////////////////////
//
//  Suspend
//
void StateManager::Suspend(Windows::ApplicationModel::SuspendingDeferral^ deferral)
{
    SwitchState(GameState::Suspended);

    Game->GameSaveManager->Suspend().then([deferral]
    {
        Log::WriteAndDisplay("OnSuspending() complete\n");
        deferral->Complete();
    });
}

///////////////////////////////////////////////////////////////////////////////
//
//  Resume
//
void StateManager::Resume()
{
    Log::Write("StateManager::Resume()\n");

    // Continue where the user left off
    RevertToPriorState();

    // Refresh LiveResources and see if the user has changed
    auto currentXuid = Game->LiveResources->GetXuid();

#ifdef _XBOX_ONE
    Game->LiveResources->Refresh();

    if (m_state == GameState::InitializeGameSaveSystem)
    {
        SwitchState(GameState::Reset);
    }
    else if (m_state == GameState::InGame)
    {
        auto currentUser = Game->LiveResources->GetUser();
        if (currentUser == nullptr && !currentXuid.empty())
        {
            // Did the last user move to a different controller?
            for (auto user : Windows::Xbox::System::User::Users)
            {
                bool xuidsEqual = _wcsicmp(user->XboxUserId->Data(), currentXuid.c_str()) == 0;
                if (xuidsEqual && user->IsSignedIn)
                {
                    // Found them...
                    currentUser = user;
                    Game->LiveResources->SetCurrentUser(user, false);
                    break;
                }
            }
        }

        auto newXuid = Game->LiveResources->GetXuid();
        bool userChanged = currentXuid != newXuid;

        auto userSignedIn = Game->LiveResources->IsUserSignedIn();

        if (!userChanged && userSignedIn)
        {
            // Resume the Connected Storage context, which will reload game saves currently in memory if needed
            Game->GameSaveManager->ResumeForUser(currentUser);
        }
        else
        {
            OnUserChanged(currentUser);
        }
    }

    Game->UpdateCurrentGamepad();
#else // UWP
    Game->LiveResources->Refresh([this, currentXuid](xbox_live_result<sign_in_result>& signInResult)
    {
        if (signInResult.err())
        {
            Log::WriteAndDisplay("Refresh signin failure (%ws): %ws\n", FormatHResult(signInResult.err().value())->Data(), utility::conversions::utf8_to_utf16(signInResult.err_message()).c_str());
            SwitchState(GameState::Reset);
        }
        else
        {
            if (signInResult.payload().status() == sign_in_status::success)
            {
                Log::Write("Refresh signin success\n");
                if (m_state == GameState::InitializeGameSaveSystem)
                {
                    SwitchState(GameState::Reset);
                }
                else if (m_state == GameState::InGame)
                {
                    auto newXuid = Game->LiveResources->GetXuid();
                    bool userChanged = currentXuid != newXuid;

                    auto currentUser = Game->LiveResources->GetUser();
                    auto userSignedIn = Game->LiveResources->IsUserSignedIn();

                    if (!userChanged && userSignedIn)
                    {
                        // Resume the Connected Storage context, which will reload game saves currently in memory if needed
                        Game->GameSaveManager->ResumeForUser(currentUser);
                    }
                    else
                    {
                        OnUserChanged(currentUser);
                    }
                }
            }
            else
            {
                Log::Write("Refresh signin requires user interaction\n");
                SwitchState(GameState::Reset);
            }
        }

        Game->UpdateCurrentGamepad();
    });
#endif
}

///////////////////////////////////////////////////////////////////////////////
//
//  ResetGame
//
void StateManager::ResetGame()
{
    Log::Write("StateManager::ResetGame()\n");

    m_screenManager->ExitAllScreens();

    if (Game->GameSaveManager != nullptr)
    {
        Game->GameSaveManager->Reset();
    }

    SwitchState(GameState::Initialize);
}

///////////////////////////////////////////////////////////////////////////////
//
//  Update
//
void StateManager::Update()
{
    auto priorState = GameState::Reset;
    auto state = GameState::Reset;
    auto pendingState = GameState::Reset;

    {
        critical_section::scoped_lock lock(m_lock);
        priorState = m_priorState;
        state = m_state;
        pendingState = m_pendingState;
    }

    if (pendingState != state)
    {
        // Perform a state switch
        {
            critical_section::scoped_lock lock(m_lock);
            Log::Write("SwitchState %ws -> %ws\n", StateToString(state)->Data(), StateToString(pendingState)->Data());
            m_priorState = state;
            state = m_state = pendingState;
        }

        switch (state)
        {
        case GameState::Reset:
            ResetGame();
            break;
        case GameState::Initialize:
            if (GameSaveManager::HasBeenInitialized)
            {
                SwitchState(GameState::AcquireUser);
            }
            else
            {
                m_screenManager->AddScreen(std::make_shared<LaunchOptionsScreen>(m_screenManager), -1);
            }
            break;
        case GameState::AcquireUser:
            m_screenManager->AddScreen(std::make_shared<AcquireUserScreen>(m_screenManager, s_autoSignIn), -1);
            s_autoSignIn = false; // only the initial signin should be automatic
            break;
        case GameState::InitializeGameSaveSystem:
            InitializeGameSaveSystem();
            break;
        case GameState::InGame:
            m_screenManager->AddScreen(std::make_shared<GameBoardScreen>(m_screenManager), Game->GetCurrentGamepadIndex());
            break;
        default:
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//  InitializeGameSaveSystem
//
void StateManager::InitializeGameSaveSystem()
{
    Game->InitializeGameSaveSystem().then([this](HRESULT result)
    {
        if (result == S_OK)
        {
            // make sure we haven't switched to a different state, e.g. suspending
            if (m_state == GameState::InitializeGameSaveSystem)
            {
                SwitchState(GameState::InGame);
            }
        }
        else
        {
            PopupError("Failed to initialize game save system (" + FormatHResult(result) + ")");
            SwitchState(GameState::AcquireUser);
        }
    });
}

///////////////////////////////////////////////////////////////////////////////
//
//  OnControllerPairingChanged
//
#ifdef _XBOX_ONE
void StateManager::OnControllerPairingChanged(Platform::Object^ /*sender*/, ControllerPairingChangedEventArgs^ args)
{
    Log::Write("OnControllerPairingChanged(%llu)\n", args->Controller->Id);

    Log::WriteAndDisplay("Controller pairing changed from %ws to %ws\n",
        FormatUserName(args->PreviousUser)->Data(),
        FormatUserName(args->User)->Data());

    if (Game->LiveResources->GetUser() != nullptr
        && args->Controller->Type->Equals(L"Windows.Xbox.Input.Gamepad"))
    {
        if (args->User != nullptr && args->User->Id == Game->LiveResources->GetUser()->Id)
        {
            Log::Write("   Updating gamepad and index\n");
            auto gamepad = dynamic_cast<IGamepad^>(args->Controller);
            Game->SetCurrentGamepad(gamepad);
        }
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////
//
//  OnControllerRemoved
//
#ifdef _XBOX_ONE
void StateManager::OnControllerRemoved(Platform::Object^ /*sender*/, Windows::Xbox::Input::ControllerRemovedEventArgs^ args)
{
    Log::Write("OnControllerRemoved(%llu)\n", args->Controller->Id);

    Log::WriteAndDisplay("Controller removed for %ws\n", FormatUserName(args->Controller->User)->Data());

    // this only matters if the controller was the current user's gamepad
    if (Game->LiveResources->GetUser() != nullptr
        && Game->GetCurrentGamepad() != nullptr
        && args->Controller->Id == Game->GetCurrentGamepad()->Id)
    {
        if (!Game->UpdateCurrentGamepad())
        {
            Game->StateManager->PopupError("The active user's controller was lost");
        }
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////
//
//  OnSignOutStarted
//
#ifdef _XBOX_ONE
void StateManager::OnSignOutStarted(SignOutStartedEventArgs^ args)
{
    Log::WriteAndDisplay("OnSignOutStarted(%ws)\n", FormatUserName(args->User)->Data());

    if (Game->LiveResources->GetUser() != nullptr
        && args->User->Id == Game->LiveResources->GetUser()->Id)
    {
        if (m_state == GameState::InGame)
        {
            auto signoutDeferral = args->GetDeferral();
            Game->GameSaveManager->OnSignOut().then([this, signoutDeferral]
            {
                signoutDeferral->Complete();
                SwitchState(GameState::Reset);
            });
        }
        else
        {
            SwitchState(GameState::Reset);
        }
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////
//
//  OnAdded
//
#ifndef _XBOX_ONE
void StateManager::OnAdded(Windows::System::UserWatcher^, Windows::System::UserChangedEventArgs^ args)
{
    Log::WriteAndDisplay("UserWatcher::OnAdded(%ws)\n", args->User->NonRoamableId->Data());
}
#endif

///////////////////////////////////////////////////////////////////////////////
//
//  OnAuthenticationStatusChanging
//
#ifndef _XBOX_ONE
void StateManager::OnAuthenticationStatusChanging(Windows::System::UserWatcher^, Windows::System::UserAuthenticationStatusChangingEventArgs^ args)
{
    Log::WriteAndDisplay("UserWatcher::OnAuthenticationStatusChanging(%ws -> %ws, %ws)\n", args->CurrentStatus.ToString()->Data(), args->NewStatus.ToString()->Data(), args->User->NonRoamableId->Data());

    if (m_state == GameState::InGame)
    {
        auto signoutDeferral = args->GetDeferral();
        Game->GameSaveManager->OnSignOut().then([this, signoutDeferral]
        {
            signoutDeferral->Complete();
            SwitchState(GameState::Reset);
        });
    }
    else
    {
        SwitchState(GameState::Reset);
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////
//
//  OnSignOutCompleted
//
#ifndef _XBOX_ONE
void StateManager::OnSignOutCompleted(ATG::XboxLiveUser user)
{
    Log::WriteAndDisplay("OnSignOutCompleted(%ws)\n", FormatUserName(user)->Data());

    if (m_state == GameState::InitializeGameSaveSystem || m_state == GameState::InGame)
    {
        SwitchState(GameState::Reset);
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////
//
//  OnUserChanged
//
void StateManager::OnUserChanged(ATG::XboxLiveUser user)
{
    Log::WriteAndDisplay("OnUserChanged(%ws)\n", FormatUserName(user)->Data());

    if (m_state == GameState::InitializeGameSaveSystem || m_state == GameState::InGame)
    {
        SwitchState(GameState::Reset);
    }
}

} // namespace GameSaveSample
