//--------------------------------------------------------------------------------------
// ErrorPopUpScreen.h
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include "ContentManager.h"
#include "GameScreen.h"

namespace GameSaveSample
{
    class ErrorPopUpScreen : public GameScreen
    {
    public:
        ErrorPopUpScreen(const std::shared_ptr<ScreenManager>& screenManager, winrt::hstring const & message = nullptr);
        virtual ~ErrorPopUpScreen();

        virtual void LoadContent() override;
        virtual void UnloadContent() override;
        virtual void HandleInput(const DirectX::InputState& inputState) override;
        virtual void Draw(float totalTime, float elapsedTime) override;

    private:
        std::shared_ptr<DirectX::Texture2D>     m_backgroundTexture;
        winrt::hstring                          m_errorMessage;
    };
}
