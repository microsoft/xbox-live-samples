//--------------------------------------------------------------------------------------
// StateManager.cpp
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

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

using namespace winrt::Windows::Xbox::Input;
using namespace winrt::Windows::Xbox::System;

namespace
{
    static bool s_autoSignIn = true;
}

namespace GameSaveSample {

winrt::hstring StateToString(GameState state)
{
    switch (state)
    {
    case GameState::Reset:
        return L"Reset";
    case GameState::Initialize:
        return L"Initialize";
    case GameState::AcquireUser:
        return L"AcquireUser";
    case GameState::InitializeGameSaveSystem:
        return L"InitializeGameSaveSystem";
    case GameState::InGame:
        return L"InGame";
    case GameState::Suspended:
        return L"Suspended";
    case GameState::Resume:
        return L"Resume";
    default:
        break;
    }
    return L"(Unhandled State)";
}

StateManager::StateManager(const std::shared_ptr<ScreenManager>& screenManager) :
    m_screenManager(screenManager),
    m_systemEventsBound(false),
    m_priorState(GameState::Reset),
    m_state(GameState::Reset),
    m_pendingState(GameState::Initialize)
{}

///////////////////////////////////////////////////////////////////////////////
//
//  PopupError
//
void StateManager::PopupError(winrt::hstring const & errorMsg, bool revertToPriorState)
{
    Log::WriteAndDisplay(L"PopupError: %s\n", errorMsg.c_str());
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
	std::lock_guard<std::mutex> lock(m_lock);
	bool currentOrPending = (m_state == state || m_pendingState == state);
    return currentOrPending;
}

///////////////////////////////////////////////////////////////////////////////
//
//  RevertToPriorState
//      Circumvents the normal SwitchState functionality - normally used to go back after an error or upon resuming
//
void StateManager::RevertToPriorState()
{
	std::lock_guard<std::mutex> lock(m_lock);
    Log::Write(L"RevertToPriorState %ws -> %ws\n", StateToString(m_state).c_str(), StateToString(m_priorState).c_str());
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
	std::lock_guard<std::mutex> lock(m_lock);

    if (newState == GameState::Suspended)
    {
        // Process this state change immediately
        Log::Write(L"SwitchState %ws -> %ws\n", StateToString(m_state).c_str(), StateToString(newState).c_str());
        m_priorState = m_state;
        m_state = GameState::Suspended;
        m_pendingState = GameState::Suspended;
        return;
    }

    if (m_state == newState)
    {
        Log::Write(L"WARNING: Attempted to switch to current state.\n");
        return;
    }

    // Set the pending state so it will pick up on the next update tick
    m_pendingState = newState;
}

///////////////////////////////////////////////////////////////////////////////
//
//  Suspend
//
void StateManager::Suspend()
{
    SwitchState(GameState::Suspended);
}

///////////////////////////////////////////////////////////////////////////////
//
//  Resume
//
void StateManager::Resume()
{
    Log::Write(L"StateManager::Resume()\n");

    // Continue where the user left off
    RevertToPriorState();

    // Refresh LiveResources and see if the user has changed
    auto currentXuid = Game->LiveResources->GetXuid();

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
            for (auto user : winrt::Windows::Xbox::System::User::Users())
            {
                bool xuidsEqual = _wcsicmp(user.XboxUserId().c_str(), currentXuid.c_str()) == 0;
                if (xuidsEqual && user.IsSignedIn())
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
            Game->GameSaveManager.ResumeForUser(currentUser);
        }
        else
        {
            OnUserChanged(currentUser);
        }
    }

    Game->UpdateCurrentGamepad();
}

///////////////////////////////////////////////////////////////////////////////
//
//  ResetGame
//
void StateManager::ResetGame()
{
    Log::Write(L"StateManager::ResetGame()\n");

    m_screenManager->ExitAllScreens();

    Game->GameSaveManager.Reset();

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
		std::lock_guard<std::mutex> lock(m_lock);
        priorState = m_priorState;
        state = m_state;
        pendingState = m_pendingState;
    }

    if (pendingState != state)
    {
        // Perform a state switch
        {
			std::lock_guard<std::mutex> lock(m_lock);
            Log::Write(L"SwitchState %ws -> %ws\n", StateToString(state).c_str(), StateToString(pendingState).c_str());
            m_priorState = state;
            state = m_state = pendingState;
        }

        switch (state)
        {
		case GameState::Reset:
			ResetGame();
			break;
        case GameState::Initialize:
            InitializeEvents();
            if (GameSaveManager::HasBeenInitialized())
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
//  InitializeEvents
//
void StateManager::InitializeEvents()
{
    Log::Write(L"StateManager::InitializeEvents()\n");

    if (m_systemEventsBound == false)
    {
		Controller::ControllerRemoved({ this, &GameSaveSample::StateManager::OnControllerRemoved });

        Controller::ControllerPairingChanged({this, &GameSaveSample::StateManager::OnControllerPairingChanged });

        m_systemEventsBound = true;
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//  InitializeGameSaveSystem
//
winrt::Windows::Foundation::IAsyncAction StateManager::InitializeGameSaveSystem()
{
	HRESULT result = co_await Game->InitializeGameSaveSystem();
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
			wchar_t errorMessage[200];
			swprintf_s(errorMessage, L"Failed to initialize game save system(%s)", FormatHResult(result).c_str());
            PopupError(errorMessage);
            SwitchState(GameState::AcquireUser);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//  OnControllerPairingChanged
//
void StateManager::OnControllerPairingChanged(winrt::Windows::Foundation::IInspectable const &, ControllerPairingChangedEventArgs const & args)
{
    Log::Write(L"OnControllerPairingChanged(%llu)\n", args.Controller().Id());

    Log::WriteAndDisplay(L"Controller pairing changed from %ws to %ws\n",
        FormatUserName(args.PreviousUser()).c_str(),
        FormatUserName(args.User()).c_str());

    if (Game->LiveResources->GetUser() != nullptr
        && wcscmp(args.Controller().Type().c_str(), L"Windows.Xbox.Input.Gamepad") == 0)
    {
        if (args.User() != nullptr && args.User().Id() == Game->LiveResources->GetUser().Id())
        {
            Log::Write(L"   Updating gamepad and index\n");
            auto controller = args.Controller();
			IGamepad* gamepad = (IGamepad*)&controller;
            Game->SetCurrentGamepad(*gamepad);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//  OnControllerRemoved
//
void StateManager::OnControllerRemoved(winrt::Windows::Foundation::IInspectable const &, winrt::Windows::Xbox::Input::ControllerRemovedEventArgs const & args)
{
    Log::Write(L"OnControllerRemoved(%llu)\n", args.Controller().Id());

    Log::WriteAndDisplay(L"Controller removed for %ws\n", FormatUserName(args.Controller().User()).c_str());

    // this only matters if the controller was the current user's gamepad
    if (Game->LiveResources->GetUser() != nullptr
        && Game->GetCurrentGamepad() != nullptr
        && args.Controller().Id() == Game->GetCurrentGamepad().Id())
    {
        if (!Game->UpdateCurrentGamepad())
        {
            Game->StateManager->PopupError(L"The active user's controller was lost");
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
//  OnSignOutStarted
//
void StateManager::OnSignOutStarted(winrt::Windows::Foundation::IInspectable const &, SignOutStartedEventArgs const & args)
{
    Log::WriteAndDisplay(L"OnSignOutStarted(%ws)\n", FormatUserName(args.User()).c_str());

    if (Game->LiveResources->GetUser() != nullptr
        && args.User().Id() == Game->LiveResources->GetUser().Id())
    {
        if (m_state == GameState::InGame)
        {
			SignOutActionAsync(args);
        }
        else
        {
            SwitchState(GameState::Reset);
        }
    }
}

winrt::Windows::Foundation::IAsyncAction StateManager::SignOutActionAsync(SignOutStartedEventArgs const & args)
{
	auto signoutDeferral = args.GetDeferral();
	co_await Game->GameSaveManager.OnSignOut();
	{
		signoutDeferral.Complete();
		SwitchState(GameState::Reset);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//  OnUserChanged
//
void StateManager::OnUserChanged(ATG::XboxLiveUser user)
{
    Log::WriteAndDisplay(L"OnUserChanged(%ws)\n", FormatUserName(user).c_str());

    if (m_state == GameState::InitializeGameSaveSystem || m_state == GameState::InGame)
    {
        SwitchState(GameState::Reset);
    }
}

} // namespace GameSaveSample
