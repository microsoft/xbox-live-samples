//--------------------------------------------------------------------------------------
// StateManager.h
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

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

    winrt::hstring StateToString(GameState state);

    class StateManager
    {
    public:
        StateManager(const std::shared_ptr<ScreenManager>& screenManager);

        void Suspend();
        void Resume();
        void ResetGame();
        void Update();

        void PopupError(winrt::hstring const & errorMsg, bool revertToPriorState = false);
        bool IsCurrentOrPendingState(GameState state);
        void RevertToPriorState();
        void SwitchState(GameState newState);

        GameState State() { return m_state; }
        bool InGame() { return m_state == GameState::InGame; }

    private:
        void InitializeEvents();
        winrt::Windows::Foundation::IAsyncAction InitializeGameSaveSystem();

    public:
        // Event Handlers
        void OnControllerPairingChanged(winrt::Windows::Foundation::IInspectable const &, winrt::Windows::Xbox::Input::ControllerPairingChangedEventArgs const & args);
        void OnControllerRemoved(winrt::Windows::Foundation::IInspectable const &, winrt::Windows::Xbox::Input::ControllerRemovedEventArgs const & args);
        void OnSignOutStarted(winrt::Windows::Foundation::IInspectable const &, winrt::Windows::Xbox::System::SignOutStartedEventArgs const & args);
		winrt::Windows::Foundation::IAsyncAction SignOutActionAsync(winrt::Windows::Xbox::System::SignOutStartedEventArgs const & args);
        void OnUserChanged(ATG::XboxLiveUser user);

    private:
        // Private Member Data
        std::mutex						m_lock;
        std::shared_ptr<ScreenManager>  m_screenManager;
        bool                            m_systemEventsBound;

        GameState m_state;
        GameState m_pendingState;
        GameState m_priorState;
    };
}
