//--------------------------------------------------------------------------------------
// GameScreen.h
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include "InputState.h"

namespace GameSaveSample
{
    enum class ScreenState
    {
        TransitionOn,
        Active,
        TransitionOff,
        Hidden
    };

    class ScreenManager;

    class GameScreen abstract
    {
    public:
        GameScreen() = delete;
        GameScreen(const std::shared_ptr<ScreenManager>& screenManager);
        virtual ~GameScreen();

        virtual void LoadContent();
        virtual void UnloadContent();
        virtual void Reset();
        virtual void Update(float totalTime, float elapsedTime, bool otherScreenHasFocus, bool coveredByOtherScreen);
        virtual void HandleInput(const DirectX::InputState& inputState);
        virtual void Draw(float totalTime, float elapsedTime);
        virtual void ExitScreen(bool immediate = false);

        bool IsPopup() const { return m_isPopup; }
        float TransitionOnTime() const { return m_transitionOnTime; }
        float TransitionOffTime() const { return m_transitionOffTime; }
        float TransitionPosition() const { return m_transitionPosition; }
        float TransitionAlpha() const { return 1.0f - m_transitionPosition; }
        ScreenState State() const { return m_state; }
        bool IsExiting() const { return m_isExiting; }
        bool IsActive() const { return !m_otherScreenHasFocus && (m_state == ScreenState::TransitionOn || m_state == ScreenState::Active); }
        std::shared_ptr<ScreenManager> Manager() { return m_screenManager; }

        // index into cached gamepad collection
        int GetControllingPlayer() const { return m_controllingPlayer; }

    protected:
        bool m_exitWhenHidden;
        bool m_isPopup;
        float m_transitionOnTime;
        float m_transitionOffTime;
        float m_transitionPosition;
        ScreenState m_state;
		std::mutex m_controllingPlayerLock;

    private:
        void UpdateControllingPlayer(int newControllingPlayer);
        bool UpdateTransition(float elapsedTime, float time, int direction);

        int m_controllingPlayer;
        bool m_isExiting;
        bool m_otherScreenHasFocus;
        std::shared_ptr<ScreenManager> m_screenManager;

        friend class ScreenManager;
    };
}
