// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

#include "GameScreen.h"
#include "InputState.h"
#include "Texture2D.h"
#include <functional>
#include <DirectXMath.h>

namespace GameSaveSample
{
    typedef std::function<void(bool adjustLeft)> MenuEntryAdjustFn;
    typedef std::function<void(int controllingPlayer)> MenuEntrySelectFn;

    class ScreenManager;

    class MenuEntry
    {
    public:
        MenuEntry(std::wstring label, MenuEntrySelectFn onSelect = nullptr, MenuEntryAdjustFn onAdjust = nullptr, std::wstring initialValue = L"");
        virtual ~MenuEntry();

        MenuEntryAdjustFn OnAdjust; // bool adjustLeft == true on left thumbstick left OR dpad left, false on left thumbstick right OR dpad right
        MenuEntrySelectFn OnSelect;

        bool m_active;
        bool m_hasValue;
        std::wstring m_label;
        std::wstring m_value;
    };

    class MenuScreen : public GameScreen
    {
    public:
        MenuScreen() = delete;
        virtual ~MenuScreen();

        virtual void LoadContent() override;
        virtual void HandleInput(const DirectX::InputState& input) override;
        virtual void Draw(float totalTime, float elapsedTime) override;

    protected:
        MenuScreen(const std::shared_ptr<ScreenManager>& screenManager);

        virtual void OnCancel() = 0;

        void ClearMenuEntries();
        virtual void ComputeMenuBounds(float viewportWidth, float viewportHeight);
        void ConfigureAsPopUpMenu();

        inline void SetCenterJustified(bool centerJustified)
        {
            m_drawCentered = centerJustified;
        }

        inline void SetMenuOffset(float xOffset, float yOffset)
        {
            m_menuOffset.x = xOffset;
            m_menuOffset.y = yOffset;
        }

        inline void SetTransitionDirections(bool transitionOnFromBelow, bool transitionOffTowardsBelow)
        {
            m_transitionOnMultiplier = transitionOnFromBelow ? 1 : -1;
            m_transitionOffMultiplier = transitionOffTowardsBelow ? 1 : -1;
        }

        bool                            m_animateSelected;
        bool                            m_menuActive;
        Windows::Foundation::Rect       m_menuBounds;
        std::vector<MenuEntry>          m_menuEntries;
        float                           m_menuSpacing;
        float                           m_menuTextScale;
        bool                            m_showCurrentUser;

    private:
        DirectX::XMFLOAT2 ComputeDrawStartPosition(float viewportWidth, float viewportHeight);

        bool                                m_drawCentered;
        DirectX::XMFLOAT2                   m_menuOffset;
        size_t                              m_selectedEntry;
        std::shared_ptr<DirectX::Texture2D> m_textureAdjustableEntryIndicatorLeft;
        int                                 m_transitionOnMultiplier;
        int                                 m_transitionOffMultiplier;
    };
}
