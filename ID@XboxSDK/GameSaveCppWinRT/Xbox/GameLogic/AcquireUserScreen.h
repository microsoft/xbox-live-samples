//--------------------------------------------------------------------------------------
// AcquireUserScreen.h
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

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
        virtual void Draw(float totalTime, float elapsedTime) override;

        virtual void HandleInput(const DirectX::InputState& input) override;

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

        void AcquireUser(int dxGamepadIndex);
		winrt::Windows::Foundation::IAsyncAction SelectUser(winrt::Windows::Xbox::Input::IGamepad gamepad);
        void SwitchUser(int dxGamepadIndex);
        void PrepareToExit(winrt::Windows::Xbox::Input::IGamepad const & gamepad);

        winrt::Windows::Foundation::IInspectable    m_dispatcher;
        AcquireUserStatus                           m_status;

        std::shared_ptr<DirectX::Texture2D>         m_backgroundTexture;
        std::shared_ptr<DirectX::Texture2D>         m_titleLogo;
    };
}
