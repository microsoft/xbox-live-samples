// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "pch.h"
#include "LaunchOptionsScreen.h"

#include "GameSaveManager.h"
#include "SampleGame.h"

using namespace DirectX;

namespace GameSaveSample {

LaunchOptionsScreen::LaunchOptionsScreen(const std::shared_ptr<ScreenManager>& screenManager) :
    MenuScreen(screenManager)
{
    m_menuEntries.push_back(MenuEntry(L"Use Full Sync Context",
        [](int)
    {
        Game->GameSaveManager->IsSyncOnDemand = false;
        Game->StateManager->SwitchState(GameState::AcquireUser);
    }));

    m_menuEntries.push_back(MenuEntry(L"Use Sync-on-Demand Context",
        [](int)
    {
        Game->GameSaveManager->IsSyncOnDemand = true;
        Game->StateManager->SwitchState(GameState::AcquireUser);
    }));
}

LaunchOptionsScreen::~LaunchOptionsScreen()
{
}

void LaunchOptionsScreen::LoadContent()
{
    MenuScreen::LoadContent();

    m_backgroundTexture = Manager()->GetContentManager()->LoadTexture(L"Assets\\blank.png");
    m_titleLogo = Manager()->GetContentManager()->LoadTexture(L"Assets\\WordGame_logo.png");
}

void LaunchOptionsScreen::Draw(float totalTime, float elapsedTime)
{
    auto spriteBatch = Manager()->GetSpriteBatch();
    auto blendStates = Manager()->GetCommonStates();
    auto viewportBounds = Manager()->GetScreenBounds();
    float viewportWidth = float(viewportBounds.right);
    float viewportHeight = float(viewportBounds.bottom);
    auto scaleMatrix = DX::GetScaleMatrixForWindow(Manager()->GetWindowBounds());

    spriteBatch->Begin(SpriteSortMode_Deferred, blendStates->NonPremultiplied(), nullptr, nullptr, nullptr, nullptr, scaleMatrix);

    // Draw the background
    RECT backgroundRectangle = { 0, 0, long(viewportWidth), long(viewportHeight) };
    spriteBatch->Draw(m_backgroundTexture->GetResourceViewTemporary(), backgroundRectangle, c_menuBackgroundColor);

    // Draw title logo
    XMFLOAT2 titleLogoPosition = XMFLOAT2(viewportWidth / 2.0f, (viewportHeight / 2.0f) - 200.f);
    XMFLOAT2 titleLogoOrigin = XMFLOAT2(m_titleLogo->Width() / 2.0f, m_titleLogo->Height() / 2.0f);
    spriteBatch->Draw(m_titleLogo->GetResourceViewTemporary(), titleLogoPosition, nullptr, Colors::White, 0.0f, titleLogoOrigin);

    spriteBatch->End();

    MenuScreen::Draw(totalTime, elapsedTime);
}

void LaunchOptionsScreen::OnCancel()
{
    // nowhere to go back to from here
}
} // namespace GameSaveSample
