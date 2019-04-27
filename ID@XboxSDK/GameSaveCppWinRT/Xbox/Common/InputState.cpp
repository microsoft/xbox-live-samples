//--------------------------------------------------------------------------------------
// InputState.cpp
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "InputState.h"

using namespace winrt::Windows::Xbox::Input;

namespace DirectX {

bool InputState::s_isAnyGamepadConnected = false;

InputState::InputState() :
    m_dpi(96.f),
    m_lastLetterKeyEvent(winrt::Windows::System::VirtualKey::None),
    m_lastNumericKeyEvent(winrt::Windows::System::VirtualKey::None),
    m_letterPressed(0),
    m_numberPressed(uint32_t(-1))
{
    m_gamePad.reset(new GamePad());
    m_keyboard.reset(new Keyboard);
    m_keyboardStateTracker.reset(new Keyboard::KeyboardStateTracker);
    m_mouse.reset(new Mouse);
    m_mouseButtonStateTracker.reset(new Mouse::ButtonStateTracker);

    GamePad::State blankState = GamePad::State();
    for (int i = 0; i < MaxInputs; i++)
    {
        LastGamePadStates[i] = CurrentGamePadStates[i] = blankState;
    }
}

void InputState::Suspend()
{
    m_gamePad->Suspend();
}

void InputState::Resume()
{
    m_gamePad->Resume();
    for (int i = 0; i < MaxInputs; i++)
    {
        m_buttonStateTracker[i].Reset();
    }
    m_keyboardStateTracker->Reset();
    m_mouseButtonStateTracker->Reset();

    m_lastLetterKeyEvent = winrt::Windows::System::VirtualKey::None;
    m_lastNumericKeyEvent = winrt::Windows::System::VirtualKey::None;
    m_letterPressed = 0;
    m_numberPressed = uint32_t(-1);
}

void InputState::SetDpi(float dpi)
{
    if (dpi != m_dpi)
    {
        m_dpi = dpi;
#ifndef _XBOX_ONE
        m_mouse->SetDpi(dpi);
#endif
    }
}

void InputState::SetWindow(IUnknown* window)
{
    UNREFERENCED_PARAMETER(window);
}

void InputState::OnKeyUp(winrt::Windows::UI::Core::KeyEventArgs const & args)
{
    if (args.VirtualKey() >= winrt::Windows::System::VirtualKey::A && args.VirtualKey() <= winrt::Windows::System::VirtualKey::Z)
    {
        m_lastLetterKeyEvent = args.VirtualKey();
    }
    else if (args.VirtualKey() >= winrt::Windows::System::VirtualKey::Number0 && args.VirtualKey() <= winrt::Windows::System::VirtualKey::Number9)
    {
        m_lastNumericKeyEvent = args.VirtualKey();
    }
}

void InputState::Update()
{
    s_isAnyGamepadConnected = false;
    for (int i = 0; i < MaxInputs; i++)
    {
        LastGamePadStates[i] = CurrentGamePadStates[i];
        CurrentGamePadStates[i] = m_gamePad->GetState(i);

        if (CurrentGamePadStates[i].IsConnected())
        {
            s_isAnyGamepadConnected = true;
            m_buttonStateTracker[i].Update(CurrentGamePadStates[i]);
        }
        else
        {
            m_buttonStateTracker[i].Reset();
        }
    }

    auto keyboardState = m_keyboard->GetState();
    m_keyboardStateTracker->Update(keyboardState);

    auto mouseButtonState = m_mouse->GetState();
    m_mouseButtonStateTracker->Update(mouseButtonState);

    m_letterPressed = 0;
    if (m_lastLetterKeyEvent != winrt::Windows::System::VirtualKey::None)
    {
        m_letterPressed = (wchar_t)m_lastLetterKeyEvent;
        m_lastLetterKeyEvent = winrt::Windows::System::VirtualKey::None;
    }

    m_numberPressed = uint32_t(-1);
    if (m_lastNumericKeyEvent != winrt::Windows::System::VirtualKey::None)
    {
        m_numberPressed = uint32_t(m_lastNumericKeyEvent) - uint32_t(winrt::Windows::System::VirtualKey::Number0);
        m_lastNumericKeyEvent = winrt::Windows::System::VirtualKey::None;
    }
}

bool InputState::IsNewButtonPress(GamepadButtons button, int controllingPlayer, int* playerIndex) const
{
    if (controllingPlayer >= 0)
    {
        if (controllingPlayer < MaxInputs)
        {
            // Read input from the specified player
            if (GetButtonState(button, controllingPlayer) == GamePad::ButtonStateTracker::ButtonState::PRESSED)
            {
                if (playerIndex)
                {
                    *playerIndex = controllingPlayer;
                }
                return true;
            }
        }

        if (playerIndex)
        {
            *playerIndex = -1;
        }
        return false;
    }
    else
    {
        // Accept input from any player
        for (int i = 0; i < MaxInputs; i++)
        {
            if (IsNewButtonPress(button, i, playerIndex))
                return true;
        }
        return false;
    }
}

GamePad::ButtonStateTracker::ButtonState InputState::GetButtonState(GamepadButtons button, int controllingPlayer) const
{
    switch (button)
    {
    case GamepadButtons::None:
        return GamePad::ButtonStateTracker::ButtonState::UP;
    case GamepadButtons::DPadUp:
        return m_buttonStateTracker[controllingPlayer].dpadUp;
    case GamepadButtons::DPadDown:
        return m_buttonStateTracker[controllingPlayer].dpadDown;
    case GamepadButtons::DPadLeft:
        return m_buttonStateTracker[controllingPlayer].dpadLeft;
    case GamepadButtons::DPadRight:
        return m_buttonStateTracker[controllingPlayer].dpadRight;
    case GamepadButtons::View:
        return m_buttonStateTracker[controllingPlayer].back;
    case GamepadButtons::Menu:
        return m_buttonStateTracker[controllingPlayer].start;
    case GamepadButtons::LeftThumbstick:
        return m_buttonStateTracker[controllingPlayer].leftStick;
    case GamepadButtons::RightThumbstick:
        return m_buttonStateTracker[controllingPlayer].rightStick;
    case GamepadButtons::LeftShoulder:
        return m_buttonStateTracker[controllingPlayer].leftShoulder;
    case GamepadButtons::RightShoulder:
        return m_buttonStateTracker[controllingPlayer].rightShoulder;
    case GamepadButtons::A:
        return m_buttonStateTracker[controllingPlayer].a;
    case GamepadButtons::B:
        return m_buttonStateTracker[controllingPlayer].b;
    case GamepadButtons::X:
        return m_buttonStateTracker[controllingPlayer].x;
    case GamepadButtons::Y:
        return m_buttonStateTracker[controllingPlayer].y;
    default:
        return GamePad::ButtonStateTracker::ButtonState::UP;
    }
}

bool InputState::IsNewKeyPress(DirectX::Keyboard::Keys key) const
{
    return m_keyboardStateTracker->IsKeyPressed(key);
}

bool InputState::IsCursorUp(int controllingPlayer, int* playerIndex) const
{
    bool newLeftThumbstickUp = false;
    if (controllingPlayer >= 0 && controllingPlayer < MaxInputs)
    {
        newLeftThumbstickUp = CurrentGamePadStates[controllingPlayer].IsLeftThumbStickUp() && !LastGamePadStates[controllingPlayer].IsLeftThumbStickUp();
    }
    else
    {
        // Accept input from any player
        for (int i = 0; i < MaxInputs; i++)
        {
            newLeftThumbstickUp = CurrentGamePadStates[i].IsLeftThumbStickUp() && !LastGamePadStates[i].IsLeftThumbStickUp();
            if (newLeftThumbstickUp)
            {
                controllingPlayer = i;
                break;
            }
        }
    }

    if (playerIndex && newLeftThumbstickUp)
        *playerIndex = controllingPlayer;

    auto kb = m_keyboard->GetState();

    return newLeftThumbstickUp
        || IsNewButtonPress(GamepadButtons::DPadUp, controllingPlayer, playerIndex)
        || m_keyboardStateTracker->pressed.Up;
}

bool InputState::IsCursorDown(int controllingPlayer, int* playerIndex) const
{
    bool newLeftThumbstickDown = false;
    if (controllingPlayer >= 0 && controllingPlayer < MaxInputs)
    {
        newLeftThumbstickDown = CurrentGamePadStates[controllingPlayer].IsLeftThumbStickDown() && !LastGamePadStates[controllingPlayer].IsLeftThumbStickDown();
    }
    else
    {
        // Accept input from any player
        for (int i = 0; i < MaxInputs; i++)
        {
            newLeftThumbstickDown = CurrentGamePadStates[i].IsLeftThumbStickDown() && !LastGamePadStates[i].IsLeftThumbStickDown();
            if (newLeftThumbstickDown)
            {
                controllingPlayer = i;
                break;
            }
        }
    }

    if (playerIndex && newLeftThumbstickDown)
        *playerIndex = controllingPlayer;

    auto kb = m_keyboard->GetState();

    return newLeftThumbstickDown
        || IsNewButtonPress(GamepadButtons::DPadDown, controllingPlayer, playerIndex)
        || m_keyboardStateTracker->pressed.Down;
}

bool InputState::IsCursorLeft(int controllingPlayer, int* playerIndex) const
{
    bool newLeftThumbstickLeft = false;
    if (controllingPlayer >= 0 && controllingPlayer < MaxInputs)
    {
        newLeftThumbstickLeft = CurrentGamePadStates[controllingPlayer].IsLeftThumbStickLeft() && !LastGamePadStates[controllingPlayer].IsLeftThumbStickLeft();
    }
    else
    {
        // Accept input from any player
        for (int i = 0; i < MaxInputs; i++)
        {
            newLeftThumbstickLeft = CurrentGamePadStates[i].IsLeftThumbStickLeft() && !LastGamePadStates[i].IsLeftThumbStickLeft();
            if (newLeftThumbstickLeft)
            {
                controllingPlayer = i;
                break;
            }
        }
    }

    if (playerIndex && newLeftThumbstickLeft)
        *playerIndex = controllingPlayer;

    auto kb = m_keyboard->GetState();

    return newLeftThumbstickLeft
        || IsNewButtonPress(GamepadButtons::DPadLeft, controllingPlayer, playerIndex)
        || m_keyboardStateTracker->pressed.Left;
}

bool InputState::IsCursorRight(int controllingPlayer, int* playerIndex) const
{
    bool newLeftThumbstickRight = false;
    if (controllingPlayer >= 0 && controllingPlayer < MaxInputs)
    {
        newLeftThumbstickRight = CurrentGamePadStates[controllingPlayer].IsLeftThumbStickRight() && !LastGamePadStates[controllingPlayer].IsLeftThumbStickRight();
    }
    else
    {
        // Accept input from any player
        for (int i = 0; i < MaxInputs; i++)
        {
            newLeftThumbstickRight = CurrentGamePadStates[i].IsLeftThumbStickRight() && !LastGamePadStates[i].IsLeftThumbStickRight();
            if (newLeftThumbstickRight)
            {
                controllingPlayer = i;
                break;
            }
        }
    }

    if (playerIndex && newLeftThumbstickRight)
        *playerIndex = controllingPlayer;

    auto kb = m_keyboard->GetState();

    return newLeftThumbstickRight
        || IsNewButtonPress(GamepadButtons::DPadRight, controllingPlayer, playerIndex)
        || m_keyboardStateTracker->pressed.Right;
}

bool InputState::IsLogScrollUp(int controllingPlayer, int* playerIndex) const
{
    bool rightThumbstickUp = false;
    if (controllingPlayer >= 0 && controllingPlayer < MaxInputs)
    {
        rightThumbstickUp = CurrentGamePadStates[controllingPlayer].IsRightThumbStickUp();
    }
    else
    {
        // Accept input from any player
        for (int i = 0; i < MaxInputs; i++)
        {
            rightThumbstickUp = CurrentGamePadStates[i].IsRightThumbStickUp();
            if (rightThumbstickUp)
            {
                controllingPlayer = i;
                break;
            }
        }
    }

    if (playerIndex && rightThumbstickUp)
    {
        *playerIndex = controllingPlayer;
    }

    auto kb = m_keyboard->GetState();

    return rightThumbstickUp
        || kb.PageUp
        || kb.OemOpenBrackets;
}

bool InputState::IsLogScrollDown(int controllingPlayer, int* playerIndex) const
{
    bool rightThumbstickDown = false;
    if (controllingPlayer >= 0 && controllingPlayer < MaxInputs)
    {
        rightThumbstickDown = CurrentGamePadStates[controllingPlayer].IsRightThumbStickDown();
    }
    else
    {
        // Accept input from any player
        for (int i = 0; i < MaxInputs; i++)
        {
            rightThumbstickDown = CurrentGamePadStates[i].IsRightThumbStickDown();
            if (rightThumbstickDown)
            {
                controllingPlayer = i;
                break;
            }
        }
    }

    if (playerIndex && rightThumbstickDown)
    {
        *playerIndex = controllingPlayer;
    }

    auto kb = m_keyboard->GetState();

    return rightThumbstickDown 
        || kb.PageDown
        || kb.OemCloseBrackets;
}

bool InputState::IsMenuSelect(int controllingPlayer, int* playerIndex) const
{
    auto kb = m_keyboard->GetState();

    return IsNewButtonPress(GamepadButtons::A, controllingPlayer, playerIndex)
        || (m_keyboardStateTracker->pressed.Enter && !(kb.LeftAlt || kb.RightAlt));
}

bool InputState::IsMenuCancel(int controllingPlayer, int* playerIndex) const
{
    return IsNewButtonPress(GamepadButtons::B, controllingPlayer, playerIndex)
        || m_keyboardStateTracker->pressed.Escape 
        || m_keyboardStateTracker->pressed.Back;
}

bool InputState::IsMenuUp(int controllingPlayer, int* playerIndex) const
{
    return IsCursorUp(controllingPlayer, playerIndex);
}

bool InputState::IsMenuDown(int controllingPlayer, int* playerIndex) const
{
    return IsCursorDown(controllingPlayer, playerIndex);
}

bool InputState::IsMenuLeft(int controllingPlayer, int* playerIndex) const
{
    return IsCursorLeft(controllingPlayer, playerIndex)
        || m_keyboardStateTracker->pressed.OemMinus;
}

bool InputState::IsMenuRight(int controllingPlayer, int* playerIndex) const
{
    return IsCursorRight(controllingPlayer, playerIndex)
        || m_keyboardStateTracker->pressed.OemPlus;
}

bool InputState::IsMenuToggle(int controllingPlayer, int* playerIndex) const
{
    return IsNewButtonPress(GamepadButtons::Menu, controllingPlayer, playerIndex)
        || m_keyboardStateTracker->pressed.Tab;
}

bool InputState::IsMouseSelect(XMINT2& mouseClick) const
{
    if (m_mouseButtonStateTracker->leftButton == Mouse::ButtonStateTracker::ButtonState::RELEASED)
    {
        auto mouseState = m_mouse->GetState();
        mouseClick.x = mouseState.x;
        mouseClick.y = mouseState.y;
        return true;
    }

    return false;
}

bool InputState::IsPauseGame(int controllingPlayer, int* playerIndex) const
{
    return IsNewButtonPress(GamepadButtons::Menu, controllingPlayer, playerIndex)
        || m_keyboardStateTracker->pressed.Tab;
}

bool InputState::IsTileScrollUp(int controllingPlayer, int * playerIndex) const
{
    bool rightThumbstickRight = false;
    if (controllingPlayer >= 0 && controllingPlayer < MaxInputs)
    {
        rightThumbstickRight = CurrentGamePadStates[controllingPlayer].IsRightThumbStickRight();
    }
    else
    {
        // Accept input from any player
        for (int i = 0; i < MaxInputs; i++)
        {
            rightThumbstickRight = CurrentGamePadStates[i].IsRightThumbStickRight();
            if (rightThumbstickRight)
            {
                controllingPlayer = i;
                break;
            }
        }
    }

    if (playerIndex && rightThumbstickRight)
        *playerIndex = controllingPlayer;

    return rightThumbstickRight;
}

bool InputState::IsTileScrollDown(int controllingPlayer, int * playerIndex) const
{
    bool rightThumbstickLeft = false;
    if (controllingPlayer >= 0 && controllingPlayer < MaxInputs)
    {
        rightThumbstickLeft = CurrentGamePadStates[controllingPlayer].IsRightThumbStickLeft();
    }
    else
    {
        // Accept input from any player
        for (int i = 0; i < MaxInputs; i++)
        {
            rightThumbstickLeft = CurrentGamePadStates[i].IsRightThumbStickLeft();
            if (rightThumbstickLeft)
            {
                controllingPlayer = i;
                break;
            }
        }
    }

    if (playerIndex && rightThumbstickLeft)
        *playerIndex = controllingPlayer;

    return rightThumbstickLeft;
}

bool InputState::IsNewRightThumbstickUp(int controllingPlayer, int* playerIndex) const
{
    bool newRightThumbstickUp = false;
    if (controllingPlayer >= 0 && controllingPlayer < MaxInputs)
    {
        newRightThumbstickUp = CurrentGamePadStates[controllingPlayer].IsRightThumbStickUp() && !LastGamePadStates[controllingPlayer].IsRightThumbStickUp();
    }
    else
    {
        // Accept input from any player
        for (int i = 0; i < MaxInputs; i++)
        {
            newRightThumbstickUp = CurrentGamePadStates[i].IsRightThumbStickUp() && !LastGamePadStates[i].IsRightThumbStickUp();
            if (newRightThumbstickUp)
            {
                controllingPlayer = i;
                break;
            }
        }
    }

    if (playerIndex && newRightThumbstickUp)
        *playerIndex = controllingPlayer;

    return newRightThumbstickUp;
}

bool InputState::IsNewRightThumbstickDown(int controllingPlayer, int* playerIndex) const
{
    bool newRightThumbstickDown = false;
    if (controllingPlayer >= 0 && controllingPlayer < MaxInputs)
    {
        newRightThumbstickDown = CurrentGamePadStates[controllingPlayer].IsRightThumbStickDown() && !LastGamePadStates[controllingPlayer].IsRightThumbStickDown();
    }
    else
    {
        // Accept input from any player
        for (int i = 0; i < MaxInputs; i++)
        {
            newRightThumbstickDown = CurrentGamePadStates[i].IsRightThumbStickDown() && !LastGamePadStates[i].IsRightThumbStickDown();
            if (newRightThumbstickDown)
            {
                controllingPlayer = i;
                break;
            }
        }
    }

    if (playerIndex && newRightThumbstickDown)
        *playerIndex = controllingPlayer;

    return newRightThumbstickDown;
}

bool InputState::IsNewRightThumbstickLeft(int controllingPlayer, int* playerIndex) const
{
    bool newRightThumbstickLeft = false;
    if (controllingPlayer >= 0 && controllingPlayer < MaxInputs)
    {
        newRightThumbstickLeft = CurrentGamePadStates[controllingPlayer].IsRightThumbStickLeft() && !LastGamePadStates[controllingPlayer].IsRightThumbStickLeft();
    }
    else
    {
        // Accept input from any player
        for (int i = 0; i < MaxInputs; i++)
        {
            newRightThumbstickLeft = CurrentGamePadStates[i].IsRightThumbStickLeft() && !LastGamePadStates[i].IsRightThumbStickLeft();
            if (newRightThumbstickLeft)
            {
                controllingPlayer = i;
                break;
            }
        }
    }

    if (playerIndex && newRightThumbstickLeft)
        *playerIndex = controllingPlayer;

    return newRightThumbstickLeft;
}

bool InputState::IsNewRightThumbstickRight(int controllingPlayer, int* playerIndex) const
{
    bool newRightThumbstickRight = false;
    if (controllingPlayer >= 0 && controllingPlayer < MaxInputs)
    {
        newRightThumbstickRight = CurrentGamePadStates[controllingPlayer].IsRightThumbStickRight() && !LastGamePadStates[controllingPlayer].IsRightThumbStickRight();
    }
    else
    {
        // Accept input from any player
        for (int i = 0; i < MaxInputs; i++)
        {
            newRightThumbstickRight = CurrentGamePadStates[i].IsRightThumbStickRight() && !LastGamePadStates[i].IsRightThumbStickRight();
            if (newRightThumbstickRight)
            {
                controllingPlayer = i;
                break;
            }
        }
    }

    if (playerIndex && newRightThumbstickRight)
        *playerIndex = controllingPlayer;

    return newRightThumbstickRight;
}

#ifdef _XBOX_ONE
winrt::Windows::Xbox::Input::IGamepad InputState::GetGamepad(int index) const
{
    if (index < -1 || index >= MaxInputs)
        return nullptr;

    uint64_t gamepadId = 0;
    if (index == -1)
    {
        for (int i = 0; i < MaxInputs; ++i)
        {
            gamepadId = m_gamePad->GetCapabilities(i).id;
            if (gamepadId > 0)
                break;
        }
    }
    else
    {
        gamepadId = m_gamePad->GetCapabilities(index).id;
    }

    if (gamepadId == 0)
        return nullptr;

    auto gamepads = winrt::Windows::Xbox::Input::Gamepad::Gamepads();
    for (uint32_t idx = 0;idx < gamepads.Size(); ++idx)
    {
        auto gamepad = gamepads.GetAt(idx);
        if (gamepad.Id() == gamepadId)
            return gamepad;
    }

    return nullptr;
}

int InputState::GetGamepadIndex(winrt::Windows::Xbox::Input::IGamepad const & gamepad) const
{
    uint64_t controllerId = gamepad.Id();

    for (int i = 0; i < MaxInputs; ++i)
    {
        if (m_gamePad->GetCapabilities(i).id == controllerId)
            return i;
    }

    return -1;
}
#endif
} // namespace DirectX
