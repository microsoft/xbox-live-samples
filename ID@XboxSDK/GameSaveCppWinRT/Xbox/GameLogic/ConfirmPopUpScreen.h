//--------------------------------------------------------------------------------------
// ConfirmPopUpScreen.h
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include "MenuScreen.h"
#include "Texture2D.h"

#pragma warning(push)
#pragma warning(disable : 4702)

#include <functional>

#pragma warning(pop)

namespace GameSaveSample
{
    typedef std::function<void(bool isConfirmed)> ConfirmChoiceFn;

    class ConfirmPopUpScreen : public GameScreen
    {
    public:
        ConfirmPopUpScreen(const std::shared_ptr<ScreenManager>& screenManager, winrt::hstring const & messageTitle, winrt::hstring const & message, ConfirmChoiceFn onChoose);
        virtual ~ConfirmPopUpScreen();

        virtual void LoadContent() override;
        virtual void HandleInput(const DirectX::InputState& inputState) override;
        virtual void Draw(float totalTime, float elapsedTime) override;

    private:
        ConfirmChoiceFn OnChoose;

        std::shared_ptr<DirectX::Texture2D>     m_backgroundTexture;
        winrt::hstring                          m_displayMessage;
        winrt::hstring                          m_displayTitle;
    };
}
