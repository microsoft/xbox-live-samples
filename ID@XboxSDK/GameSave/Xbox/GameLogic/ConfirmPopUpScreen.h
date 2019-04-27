// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

#include "MenuScreen.h"
#include "Texture2D.h"
#include <functional>

namespace GameSaveSample
{
    typedef std::function<void(bool isConfirmed)> ConfirmChoiceFn;

    class ConfirmPopUpScreen : public GameScreen
    {
    public:
        ConfirmPopUpScreen(const std::shared_ptr<ScreenManager>& screenManager, Platform::String^ messageTitle, Platform::String^ message, ConfirmChoiceFn onChoose);
        virtual ~ConfirmPopUpScreen();

        virtual void LoadContent() override;
        virtual void HandleInput(const DirectX::InputState& inputState) override;
        virtual void Draw(float totalTime, float elapsedTime) override;

    private:
        ConfirmChoiceFn OnChoose;

        std::shared_ptr<DirectX::Texture2D>     m_backgroundTexture;
        Platform::String^                       m_displayMessage;
        Platform::String^                       m_displayTitle;
    };
}
