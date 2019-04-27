//--------------------------------------------------------------------------------------
// MenuScreen.cpp
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "MenuScreen.h"

#include "InputState.h"
#include "SampleGame.h"
#include "ScreenManager.h"

#include <math.h>

using namespace Concurrency;
using namespace DirectX;

namespace GameSaveSample {

namespace
{
    const DirectX::XMVECTORF32 c_menuItemColor = { 230.f / 255.f, 178.f / 255.f, 138.f / 255.f, 1.f };
}

//
// MenuEntry
//
MenuEntry::MenuEntry(std::wstring label, MenuEntrySelectFn onSelect, MenuEntryAdjustFn onAdjust, std::wstring initialValue) :
    OnAdjust(onAdjust),
    OnSelect(onSelect),
    m_active(true),
    m_hasValue(onAdjust != nullptr),
    m_label(label),
    m_value(initialValue)
{
}

MenuEntry::~MenuEntry()
{
}


//
// MenuScreen
//
MenuScreen::MenuScreen(const std::shared_ptr<ScreenManager>& screenManager) : GameScreen(screenManager),
    m_animateSelected(true),
    m_menuActive(true),
    m_menuBounds(winrt::Windows::Foundation::Rect()),
    m_menuOffset(XMFLOAT2(0, 0)),
    m_menuSpacing(1.0f),
    m_menuTextScale(1.0f),
    m_showCurrentUser(true),
    m_selectedEntry(0),
    m_transitionOnMultiplier(1),
    m_transitionOffMultiplier(1)
{
    SetCenterJustified(true);
}

MenuScreen::~MenuScreen()
{
}

void MenuScreen::LoadContent()
{
}

void MenuScreen::HandleInput(const DirectX::InputState& input)
{
    int controllingPlayer = -1;
    {
		std::lock_guard<std::mutex> playerLock(m_controllingPlayerLock);
        controllingPlayer = GetControllingPlayer();
    }

    if (input.IsMenuCancel(controllingPlayer, nullptr))
    {
        OnCancel();
    }

    // the remaining input handling code is only relevant if the menu is active
    if (!m_menuActive)
        return;

    int selectingPlayer = -1;

    if (input.IsMenuUp(controllingPlayer, nullptr))
    {
        // size_t doesn't go negative; it rolls over
        m_selectedEntry--;
        if (m_selectedEntry >= m_menuEntries.size())
            m_selectedEntry = m_menuEntries.size() - 1;
    }
    else if (input.IsMenuDown(controllingPlayer, nullptr))
    {
        m_selectedEntry++;
        if (m_selectedEntry >= m_menuEntries.size())
            m_selectedEntry = 0;
    }
    else if (input.IsMenuSelect(controllingPlayer, &selectingPlayer))
    {
        if (m_selectedEntry < m_menuEntries.size()
            && m_menuEntries[m_selectedEntry].OnSelect != nullptr
            && m_menuEntries[m_selectedEntry].m_active)
        {
            m_menuEntries[m_selectedEntry].OnSelect(selectingPlayer);
        }
    }
    else if (input.IsMenuLeft(controllingPlayer, nullptr))
    {
        if (m_selectedEntry < m_menuEntries.size()
            && m_menuEntries[m_selectedEntry].OnAdjust != nullptr
            && m_menuEntries[m_selectedEntry].m_active)
        {
            m_menuEntries[m_selectedEntry].OnAdjust(true);
        }
    }
    else if (input.IsMenuRight(controllingPlayer, nullptr))
    {
        if (m_selectedEntry < m_menuEntries.size()
            && m_menuEntries[m_selectedEntry].OnAdjust != nullptr
            && m_menuEntries[m_selectedEntry].m_active)
        {
            m_menuEntries[m_selectedEntry].OnAdjust(false);
        }
    }
}

void MenuScreen::Draw(float totalTime, float elapsedTime)
{
    UNREFERENCED_PARAMETER(elapsedTime);

    auto viewportBounds = Manager()->GetScreenBounds();
    float viewportWidth = float(viewportBounds.right);
    float viewportHeight = float(viewportBounds.bottom);

    XMFLOAT2 position = ComputeDrawStartPosition(viewportWidth, viewportHeight);

    // Make the menu slide into place during transitions, using a
    // power curve to make things look more interesting (this makes
    // the movement slow down as it nears the end).
    float transitionOffset = pow(TransitionPosition(), 2);

    if (State() == ScreenState::TransitionOn)
        position.y += transitionOffset * (256 * m_transitionOnMultiplier);
    else
        position.y += transitionOffset * (512 * m_transitionOffMultiplier);

    // Draw each menu entry in turn.
    auto spriteBatch = Manager()->GetSpriteBatch();
    auto font = Manager()->GetSpriteFont();
    auto common = Manager()->GetCommonStates();
    auto scaleMatrix = DX::GetScaleMatrixForWindow(Manager()->GetWindowBounds());

    spriteBatch->Begin(SpriteSortMode_Deferred, common->NonPremultiplied(), nullptr, nullptr, nullptr, nullptr, scaleMatrix);

    // Advance starting position halfway down the height of the first line, since this is where the draw origin is.
    position.y += font->GetLineSpacing() / 2.0f;

    for (size_t i = 0; i < m_menuEntries.size(); i++)
    {
        if (m_menuEntries[i].m_label.empty())
            continue;

        XMVECTORF32 color = c_menuItemColor;
        if (!m_menuEntries[i].m_active)
        {
            color = Colors::Gray;
        }
        float adjustedScale = 1.f;

        if (m_menuActive && IsActive() && (i == m_selectedEntry))
        {
            // The selected entry is white (or gray, if inactive), and has an animating size (if enabled).
            if (m_menuEntries[i].m_active)
            {
                color = Colors::White;

                if (m_animateSelected)
                {
                    float pulsate = sin(totalTime * 6.0f) + 1.0f;
                    adjustedScale += pulsate * 0.05f;
                }
            }
            else
            {
                color = Colors::DarkGray;
            }
        }

        // Modify the alpha to fade text out during transitions.
        color.f[3] = TransitionAlpha();

        // Draw text, centered on the middle of each line.
        std::wstring menuLabel = m_menuEntries[i].m_label;
        if (m_menuEntries[i].m_hasValue)
        {
            menuLabel.append(L" ");
            menuLabel += m_menuEntries[i].m_value;
        }
        XMVECTOR size = font->MeasureString(menuLabel.data());
        XMFLOAT2 origin = XMFLOAT2(m_drawCentered ? XMVectorGetX(size) / 2.0f : 0, font->GetLineSpacing() / 2.0f);
        font->DrawString(spriteBatch.get(), menuLabel.data(), position, color, 0, origin, adjustedScale, SpriteEffects_None, 0);

        position.y += font->GetLineSpacing() * m_menuSpacing;
    }

    // Draw current user at bottom right.
    if (m_showCurrentUser && Game->LiveResources->IsUserSignedIn())
    {
        auto currentUser = Game->LiveResources->GetUser();
        winrt::hstring displayName = FormatUserName(currentUser, false, true);
        position = XMFLOAT2(m_menuBounds.X + m_menuBounds.Width * 0.95f, m_menuBounds.Y + m_menuBounds.Height * 0.9f);
        float userNameWidth = XMVectorGetX(font->MeasureString(displayName.c_str()));
        XMFLOAT2 origin = XMFLOAT2(userNameWidth, font->GetLineSpacing() / 2.0f);
        font->DrawString(spriteBatch.get(), displayName.c_str(), position, Colors::White, 0, origin);
    }

    spriteBatch->End();
}

void MenuScreen::ClearMenuEntries()
{
    m_selectedEntry = 0;
    m_menuEntries.clear();
}

void MenuScreen::ComputeMenuBounds(float viewportWidth, float viewportHeight)
{
	m_menuBounds = winrt::Windows::Foundation::Rect{ 0, viewportHeight * 0.55f, viewportWidth, viewportHeight * 0.45f };
}

void MenuScreen::ConfigureAsPopUpMenu()
{
    m_isPopup = true;
    SetCenterJustified(false);
    m_animateSelected = false;
    m_showCurrentUser = false;
}

XMFLOAT2 MenuScreen::ComputeDrawStartPosition(float viewportWidth, float viewportHeight)
{
    ComputeMenuBounds(viewportWidth, viewportHeight);

    if (m_drawCentered)
    {
        return XMFLOAT2(m_menuOffset.x + m_menuBounds.Width / 2.0f, m_menuOffset.y + m_menuBounds.Y);
    }
    else
    {
        return XMFLOAT2(m_menuOffset.x + m_menuBounds.X, m_menuOffset.y + m_menuBounds.Y);
    }
}
} // namespace GameSaveSample
