// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "pch.h"
#include "ConfirmPopUpScreen.h"

#include "ContentManager.h"
#include "InputState.h"
#include "ScreenManager.h"

using namespace DirectX;

namespace
{
    const XMVECTORF32 BackgroundColor = Colors::DarkSlateGray;
}

namespace GameSaveSample {

ConfirmPopUpScreen::ConfirmPopUpScreen(const std::shared_ptr<ScreenManager>& screenManager, Platform::String^ messageTitle, Platform::String^ message, ConfirmChoiceFn onChoose) :
    GameScreen(screenManager),
    OnChoose(onChoose),
    m_displayMessage(message),
    m_displayTitle(messageTitle)
{
    m_exitWhenHidden = false;
    m_isPopup = true;
}

ConfirmPopUpScreen::~ConfirmPopUpScreen()
{
}

void ConfirmPopUpScreen::LoadContent()
{
    auto screenManager = this->Manager();
    m_backgroundTexture = screenManager->GetContentManager()->LoadTexture(L"Assets\\blank.png");
}

void ConfirmPopUpScreen::HandleInput(const DirectX::InputState& inputState)
{
    if (inputState.IsMenuSelect(-1, nullptr))
    {
        OnChoose(true);
        ExitScreen();
    }
    else if (inputState.IsMenuCancel(-1, nullptr))
    {
        OnChoose(false);
        ExitScreen();
    }
}

void ConfirmPopUpScreen::Draw(float totalTime, float elapsedTime)
{
    UNREFERENCED_PARAMETER(totalTime);
    UNREFERENCED_PARAMETER(elapsedTime);

    auto screenManager = this->Manager();
    auto spriteBatch = screenManager->GetSpriteBatch();
    auto spriteFont = screenManager->GetSpriteFont();
    auto blendStates = screenManager->GetCommonStates();
    auto viewportBounds = screenManager->GetScreenBounds();
    float viewportWidth = float(viewportBounds.right);
    float viewportHeight = float(viewportBounds.bottom);
    auto scaleMatrix = DX::GetScaleMatrixForWindow(screenManager->GetWindowBounds());

    Platform::String^ confirmPrompt = "Press (A) to Confirm; (B) to Cancel";
#ifndef _XBOX_ONE
    if (!InputState::IsAnyGamepadConnected())
    {
        confirmPrompt = "Press Enter to Confirm; Backspace to Cancel";
    }
#endif
    XMFLOAT2 messageOrigin = XMFLOAT2(0, 0);
    float messageTitleFontScale = 1.25f;

    // calculate position and size of display message region
    XMVECTOR messageTitleSize = spriteFont->MeasureString(m_displayTitle->Data());
    XMVECTOR messageSize = spriteFont->MeasureString(m_displayMessage->Data());
    XMVECTOR confirmSize = spriteFont->MeasureString(confirmPrompt->Data());

    float maxTextWidth = XMVectorGetX(messageSize);
    maxTextWidth = std::max(maxTextWidth, XMVectorGetX(messageTitleSize) * messageTitleFontScale);
    maxTextWidth = std::max(maxTextWidth, XMVectorGetX(confirmSize));

    XMFLOAT2 textArea = XMFLOAT2(maxTextWidth,
        (XMVectorGetY(messageTitleSize) * messageTitleFontScale)
        + XMVectorGetY(messageSize)
        + XMVectorGetY(confirmSize)
        + spriteFont->GetLineSpacing() * 2);

    XMFLOAT2 textPosition = XMFLOAT2(viewportWidth / 2.0f - textArea.x / 2.0f, viewportHeight / 2.0f - textArea.y / 2.0f);

    // create a rectangle representing the screen dimensions of the display message background rectangle
    long rectangleWidth = long(std::min(textArea.x + 100.0f, viewportWidth));
    long rectangleHeight = long(std::min(textArea.y + 100.0f, viewportHeight));
    long rectangleLeft = long(std::max(0.0f, textPosition.x - 50.0f));
    long rectangleTop = long(std::max(0.0f, textPosition.y - 50.0f));
    RECT backgroundRectangle = { rectangleLeft, rectangleTop, rectangleLeft + rectangleWidth, rectangleTop + rectangleHeight };

    spriteBatch->Begin(SpriteSortMode_Deferred, blendStates->NonPremultiplied(), nullptr, nullptr, nullptr, nullptr, scaleMatrix);

    // draw a background color for the popup
    spriteBatch->Draw(m_backgroundTexture->GetResourceViewTemporary(), backgroundRectangle, BackgroundColor);

    // draw message title near the top of the popup
    spriteFont->DrawString(spriteBatch.get(), m_displayTitle->Data(), textPosition, Colors::White, 0, messageOrigin, messageTitleFontScale);

    // draw message below the title
    textPosition.y += spriteFont->GetLineSpacing() * messageTitleFontScale * 2.0f;
    spriteFont->DrawString(spriteBatch.get(), m_displayMessage->Data(), textPosition, Colors::Yellow, 0, messageOrigin);

    // draw choice prompt below the message
    textPosition.y += XMVectorGetY(messageSize) + spriteFont->GetLineSpacing();
    spriteFont->DrawString(spriteBatch.get(), confirmPrompt->Data(), textPosition, Colors::Yellow, 0, messageOrigin);

    spriteBatch->End();
}
} // namespace GameSaveSample
