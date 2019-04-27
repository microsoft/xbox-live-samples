// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "pch.h"
#include "ScreenManager.h"

#include "ContentManager.h"
#include "InputState.h"
#include "SampleGame.h"

#include <CommonStates.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <algorithm>

using namespace Concurrency;
using namespace DirectX;
using namespace Platform;

#ifdef _XBOX_ONE
using namespace Windows::Xbox::Input;
#else
using namespace Windows::Gaming::Input;
#endif

namespace GameSaveSample {

ScreenManager::ScreenManager() :
    m_isDebugOverlayOn(false),
    m_screenStackCleared(false)
{
}

void ScreenManager::CreateDeviceDependentResources(const std::shared_ptr<DX::DeviceResources>& deviceResources)
{
    m_deviceResources = deviceResources;
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto device = m_deviceResources->GetD3DDevice();

    // Create the content manager
    m_contentManager = std::make_shared<ContentManager>(m_deviceResources);

    // Create the DirectXTK resources
    m_commonStates = std::make_shared<CommonStates>(device);
    m_spriteBatch = std::make_shared<SpriteBatch>(context);

    m_spriteFont = std::make_shared<SpriteFont>(device, L"Assets\\Fonts\\SegoeUI_24_NP.spritefont");
	m_spriteFont->SetDefaultCharacter(L'*');

    m_debugFont = std::make_shared<SpriteFont>(device, L"Assets\\Fonts\\Consolas_12_NP.spritefont");
    m_debugFont->SetDefaultCharacter(L'*');
}

void ScreenManager::CreateWindowSizeDependentResources()
{
#ifndef _XBOX_ONE
    m_spriteBatch->SetRotation(m_deviceResources->GetRotation());
#endif
}

void ScreenManager::OnDeviceLost()
{
    m_contentManager.reset();
    m_commonStates.reset();
    m_spriteBatch.reset();
    m_spriteFont.reset();
    m_debugFont.reset();
}

void ScreenManager::ExitAllScreens()
{
    while (m_screens.size() > 0)
    {
        auto screen = m_screens.back();
        screen->ExitScreen(true);
    }
    m_screenStackCleared = true;
}

void ScreenManager::AddScreen(const std::shared_ptr<GameScreen>& screen, int controllingPlayer)
{
    critical_section::scoped_lock lock(m_screenLock);

    screen->m_controllingPlayer = controllingPlayer;
    screen->m_isExiting = false;

    if (!screen->IsPopup() && m_screens.size() > 0)
    {
        // insert the new screen underneath any existing popup screens
        ScreensIterator it = m_screens.begin();
        for (; it != m_screens.end(); ++it)
        {
            if ((*it)->IsPopup())
                break;
        }
        m_screens.insert(it, screen);
    }
    else
    {
        // otherwise just tack on the new screen at the end (top) of the list
        m_screens.push_back(screen);
    }

    // Allow the screen to load content now that it's being added to the screen manager
    screen->LoadContent();
}

void ScreenManager::RemoveScreen(const std::shared_ptr<GameScreen>& screen)
{
    // Shell out to our private helper since it's going to do the same thing
    RemoveScreen(screen.get());
}

void ScreenManager::UpdateControllingPlayer(int newControllingPlayer)
{
    critical_section::scoped_lock lock(m_screenLock);

    std::for_each(m_screens.begin(), m_screens.end(), [&](const std::shared_ptr<GameScreen>& screen)
    {
        if (screen->m_controllingPlayer != -1)
            screen->UpdateControllingPlayer(newControllingPlayer);
    });
}

void ScreenManager::Update(DX::StepTimer const& timer)
{
    float totalTime = float(timer.GetTotalSeconds());
    float elapsedTime = float(timer.GetElapsedSeconds());

    if (Game->InputManager->IsNewButtonPress(GamepadButtons::View, -1) || Game->InputManager->IsNewKeyPress(Keyboard::Keys::OemTilde))
    {
        m_isDebugOverlayOn = !m_isDebugOverlayOn;
    }

    // Make a copy of the master screen list, to avoid confusion if the process of updating one screen adds or removes others
    std::vector<std::shared_ptr<GameScreen>> screensToUpdate;
    {
        critical_section::scoped_lock lock(m_screenLock);
        screensToUpdate = m_screens;
    }

    bool otherScreenHasFocus = false;
    bool coveredByOtherScreen = false; // used to tell non-popup screens that they are covered by another non-popup screen
    bool coveredByPopup = false; // used to tell other popup screens that they are covered by a popup above them
    m_screenStackCleared = false;

    // Iterate the screens in reverse order so the last screen added is considered the top of the "stack"
    for (ReverseScreensIterator itr = screensToUpdate.rbegin(); itr != screensToUpdate.rend(); itr++)
    {
        std::shared_ptr<GameScreen> screen = (*itr);
        bool isPopup = screen->m_isPopup;

        // Update the screen
        screen->Update(totalTime, elapsedTime, otherScreenHasFocus, isPopup ? coveredByPopup : coveredByOtherScreen);

        // Update this value for the next screen
        coveredByPopup = isPopup;

        if (screen->m_state == ScreenState::TransitionOn || screen->m_state == ScreenState::Active)
        {
            // If this is the first active screen we came across, give it a chance to handle input
            if (!otherScreenHasFocus)
            {
                screen->HandleInput(*(Game->InputManager));
                otherScreenHasFocus = true;
            }

            // If this is an active non-popup, inform any subsequent screens that they are covered by it
            if (!isPopup)
            {
                coveredByOtherScreen = true;
            }
        }

        if (m_screenStackCleared)
        {
            // ExitAllScreens() was called by the current screen so don't process any more screens
            break;
        }
    }

    // If the background screen needed updating, we would do it here. So far, the Update() is unnecessary.
}

void ScreenManager::Render(DX::StepTimer const& timer)
{
    float totalTime = float(timer.GetTotalSeconds());
    float elapsedTime = float(timer.GetElapsedSeconds());

    // Make a copy of the master screen list so we can loop through and draw the screens even if another thread alters the collection during the draw
    std::vector<std::shared_ptr<GameScreen>> screensToRender;
    {
        critical_section::scoped_lock lock(m_screenLock);
        screensToRender = m_screens;
    }

    std::for_each(screensToRender.begin(), screensToRender.end(), [&](const std::shared_ptr<GameScreen>& screen)
    {
        if (screen->m_state != ScreenState::Hidden)
            screen->Draw(totalTime, elapsedTime);
    });

    if (m_isDebugOverlayOn)
    {
        auto viewportSize = GetScreenBounds();
        auto scaleMatrix = DX::GetScaleMatrixForWindow(GetWindowBounds());
        m_spriteBatch->Begin(SpriteSortMode_Deferred, m_commonStates->NonPremultiplied(), nullptr, nullptr, nullptr, nullptr, scaleMatrix);

        float viewportWidth = float(viewportSize.right);
        float viewportHeight = float(viewportSize.bottom);

        // draw current state at the top left of the screen
        XMFLOAT2 position = XMFLOAT2(viewportWidth * 0.05f, viewportHeight * 0.05f);
        XMFLOAT2 origin = XMFLOAT2(0, m_spriteFont->GetLineSpacing() / 2.0f);
        String^ state = StateToString(Game->StateManager->State);
        m_spriteFont->DrawString(m_spriteBatch.get(), state->Data(), position, Colors::White, 0, origin);

        if (Game->GameSaveManager != nullptr)
        {
            // draw game save index update count at the top middle of the screen
            position = XMFLOAT2(viewportWidth * 0.5f, viewportHeight * 0.05f);
            String^ updateCount = "Index Update #" + Game->GameSaveManager->IndexUpdateCount.ToString();
            auto countDrawWidth = m_spriteFont->MeasureString(updateCount->Data());
            origin = XMFLOAT2(XMVectorGetX(countDrawWidth) / 2.0f, m_spriteFont->GetLineSpacing() / 2.0f);
            m_spriteFont->DrawString(m_spriteBatch.get(), updateCount->Data(), position, Colors::White, 0, origin);
        }

        // draw FPS at the top right of the screen
        position = XMFLOAT2(viewportWidth * 0.95f, viewportHeight * 0.05f);
        String^ fps = timer.GetFramesPerSecond().ToString() + " FPS";
        auto fpsDrawWidth = m_spriteFont->MeasureString(fps->Data());
        origin = XMFLOAT2(XMVectorGetX(fpsDrawWidth), m_spriteFont->GetLineSpacing() / 2.0f);
        m_spriteFont->DrawString(m_spriteBatch.get(), fps->Data(), position, Colors::White, 0, origin);

        m_spriteBatch->End();
    }
}

void ScreenManager::RemoveScreen(GameScreen* screen)
{
    critical_section::scoped_lock lock(m_screenLock);

    // Find the screen in our collection
    for (ScreensIterator itr = m_screens.begin(); itr != m_screens.end(); ++itr)
    {
        if ((*itr).get() == screen)
        {
            // Let the screen unload any content if it wants
            screen->UnloadContent();

            // Remove it from the vector
            m_screens.erase(itr);
            return;
        }
    }
}
} // namespace GameSaveSample
