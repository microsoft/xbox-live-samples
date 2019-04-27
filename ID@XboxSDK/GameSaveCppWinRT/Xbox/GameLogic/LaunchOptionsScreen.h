//--------------------------------------------------------------------------------------
// LaunchOptionsScreen.h
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include "MenuScreen.h"
#include "Texture2D.h"

namespace GameSaveSample
{
    class LaunchOptionsScreen : public MenuScreen
    {
    public:
        LaunchOptionsScreen(const std::shared_ptr<ScreenManager>& screenManager);
	    virtual ~LaunchOptionsScreen();

        virtual void LoadContent() override;
        virtual void Draw(float totalTime, float elapsedTime) override;

    protected:
	    virtual void OnCancel() override;

    private:
        std::shared_ptr<DirectX::Texture2D>     m_backgroundTexture;
        std::shared_ptr<DirectX::Texture2D>     m_titleLogo;
    };
}
