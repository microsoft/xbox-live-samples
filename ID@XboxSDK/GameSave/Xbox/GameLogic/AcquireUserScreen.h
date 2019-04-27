// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

#include "MenuScreen.h"
#include "Texture2D.h"

namespace GameSaveSample
{
    class AcquireUserScreen : public MenuScreen
    {
    public:
        AcquireUserScreen(const std::shared_ptr<ScreenManager>& screenManager, bool autoSignIn);
        virtual ~AcquireUserScreen();

        virtual void LoadContent() override;
#ifndef _XBOX_ONE
        virtual void Update(float totalTime, float elapsedTime, bool otherScreenHasFocus, bool coveredByOtherScreen) override;
#endif
        virtual void Draw(float totalTime, float elapsedTime) override;

#ifdef _XBOX_ONE
        virtual void HandleInput(const DirectX::InputState& input) override;
#endif

    protected:
        virtual void OnCancel() override;

    private:
        enum class AcquireUserStatus
        {
            SigningIn,
            NeedsUserInteraction,
            Ready,
            Exiting
        };

        enum class SigninMethod
        {
            Silent,
            Normal
        };

#ifdef _XBOX_ONE
        void AcquireUser(int dxGamepadIndex);
        void SelectUser(Windows::Xbox::Input::IGamepad^ gamepad);
        void SwitchUser(int dxGamepadIndex);
        void PrepareToExit(Windows::Xbox::Input::IGamepad^ gamepad);
#else
        Concurrency::task<void> AcquireUser(int dxGamepadIndex);
        void SignIn(SigninMethod method);
        void UpdateMenu(AcquireUserStatus status);
#endif

        Platform::Object^                       m_dispatcher;
        Concurrency::critical_section           m_menuLock;
        AcquireUserStatus                       m_status;

        std::shared_ptr<DirectX::Texture2D>     m_backgroundTexture;
        std::shared_ptr<DirectX::Texture2D>     m_titleLogo;
    };
}
