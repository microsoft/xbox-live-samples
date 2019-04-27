// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

#include "ContentManager.h"
#include "GameScreen.h"

namespace GameSaveSample
{
    class ErrorPopUpScreen : public GameScreen
    {
    public:
        ErrorPopUpScreen(const std::shared_ptr<ScreenManager>& screenManager, Platform::String^ message = nullptr);
        virtual ~ErrorPopUpScreen();

        virtual void LoadContent() override;
        virtual void UnloadContent() override;
        virtual void HandleInput(const DirectX::InputState& inputState) override;
        virtual void Draw(float totalTime, float elapsedTime) override;

    private:
        std::shared_ptr<DirectX::Texture2D>     m_backgroundTexture;
        Platform::String^                       m_errorMessage;
    };
}
