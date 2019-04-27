// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

#ifdef _XBOX_ONE
#include "LiveResourcesXDK.h"
#else
#include "LiveResourcesUWP.h"
#endif
#include "ScreenManager.h"

namespace GameSaveSample
{
    enum class GameState
    {
        Reset,
        Initialize,
        AcquireUser,
        InitializeGameSaveSystem,
        InGame,
        Suspended,
        Resume
    };

    Platform::String^ StateToString(GameState state);

    ref class StateManager sealed
    {
    internal:
        StateManager(const std::shared_ptr<ScreenManager>& screenManager);

        void Suspend(Windows::ApplicationModel::SuspendingDeferral^ deferral);
        void Resume();
        void ResetGame();
        void Update();

        void PopupError(Platform::String^ errorMsg, bool revertToPriorState = false);
        bool IsCurrentOrPendingState(GameState state);
        void RevertToPriorState();
        void SwitchState(GameState newState);

        property GameState State
        {
            GameState get() { return m_state; }
        }

        property bool InGame
        {
            bool get()
            {
                return m_state == GameState::InGame;
            }
        }

    private:
        void InitializeGameSaveSystem();

    internal:
        // Event Handlers
#ifdef _XBOX_ONE
        void OnControllerPairingChanged(Platform::Object^ sender, Windows::Xbox::Input::ControllerPairingChangedEventArgs^ args);
        void OnControllerRemoved(Platform::Object^ sender, Windows::Xbox::Input::ControllerRemovedEventArgs^ args);
        void OnSignOutStarted(Windows::Xbox::System::SignOutStartedEventArgs^ args);
#else
        static void OnAdded(Windows::System::UserWatcher^ sender, Windows::System::UserChangedEventArgs^ args);
        void OnAuthenticationStatusChanging(Windows::System::UserWatcher^ sender, Windows::System::UserAuthenticationStatusChangingEventArgs^ args);
        void OnSignOutCompleted(ATG::XboxLiveUser user);
#endif
        void OnUserChanged(ATG::XboxLiveUser user);

    private:
        // Private Member Data
        Concurrency::critical_section   m_lock;
        std::shared_ptr<ScreenManager>  m_screenManager;

        GameState m_state;
        GameState m_pendingState;
        GameState m_priorState;
    };
}
