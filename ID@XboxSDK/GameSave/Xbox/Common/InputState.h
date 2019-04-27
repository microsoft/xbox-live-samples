// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

#include <GamePad.h>
#include <Keyboard.h>
#include <Mouse.h>
#include <array>

// Contains all game input in one central location and exposes helper functions for working with the input
namespace DirectX {
    class InputState
    {
    public:
        static const int MaxInputs = DirectX::GamePad::MAX_PLAYER_COUNT;

        InputState();

        // Returns true if any gamepad is connected
        static inline bool IsAnyGamepadConnected() { return s_isAnyGamepadConnected; }

        // Handle PLM and window events
        void Suspend();
        void Resume();
        void SetDpi(float dpi);
        void SetWindow(IUnknown* window);
        void OnKeyUp(Windows::UI::Core::KeyEventArgs^ args);

        // Updates the current state of the input
        void Update();

#ifdef _XBOX_ONE
        // Checks if a button was just pressed this frame.
        //   button - The button to test.
        //   controllingPlayer - The player index to test (0-7) or -1 to test all gamepads.
        //   playerIndex - (Optional) Returns the index of the gamepad that pressed the button or -1 if none.
        bool IsNewButtonPress(Windows::Xbox::Input::GamepadButtons button, int controllingPlayer = -1, int* playerIndex = nullptr) const;

        DirectX::GamePad::ButtonStateTracker::ButtonState GetButtonState(Windows::Xbox::Input::GamepadButtons button, int controllingPlayer) const;
#else
        // Checks if a button was just pressed this frame
        //   button - The button to test.
        //   controllingPlayer - The player index to test (0-7) or -1 to test all gamepads.
        //   playerIndex - (Optional) Returns the index of the gamepad that pressed the button or -1 if none.
        bool IsNewButtonPress(Windows::Gaming::Input::GamepadButtons button, int controllingPlayer = -1, int* playerIndex = nullptr) const;

        DirectX::GamePad::ButtonStateTracker::ButtonState GetButtonState(Windows::Gaming::Input::GamepadButtons button, int controllingPlayer) const;
#endif

        inline wchar_t GetLetterPressed() const { return m_letterPressed; }
        inline uint32_t GetNumberPressed() const { return m_numberPressed; }
        bool IsNewKeyPress(DirectX::Keyboard::Keys key) const;

        // Helper functions for standard game functions that call into IsNewButtonPress with various standard inputs
        bool IsCursorUp(int controllingPlayer, int* playerIndex) const;
        bool IsCursorDown(int controllingPlayer, int* playerIndex) const;
        bool IsCursorLeft(int controllingPlayer, int* playerIndex) const;
        bool IsCursorRight(int controllingPlayer, int* playerIndex) const;
        bool IsLogScrollUp(int controllingPlayer = -1, int* playerIndex = nullptr) const;
        bool IsLogScrollDown(int controllingPlayer = -1, int* playerIndex = nullptr) const;
        bool IsMenuSelect(int controllingPlayer = -1, int* playerIndex = nullptr) const;
        bool IsMenuCancel(int controllingPlayer = -1, int* playerIndex = nullptr) const;
        bool IsMenuUp(int controllingPlayer = -1, int* playerIndex = nullptr) const;
        bool IsMenuDown(int controllingPlayer = -1, int* playerIndex = nullptr) const;
        bool IsMenuLeft(int controllingPlayer = -1, int* playerIndex = nullptr) const;
        bool IsMenuRight(int controllingPlayer = -1, int* playerIndex = nullptr) const;
        bool IsMenuToggle(int controllingPlayer = -1, int* playerIndex = nullptr) const;
        bool IsMouseSelect(DirectX::XMINT2& mouseClick) const;
        bool IsPauseGame(int controllingPlayer, int* playerIndex) const;
        bool IsTileScrollUp(int controllingPlayer = -1, int* playerIndex = nullptr) const;
        bool IsTileScrollDown(int controllingPlayer = -1, int* playerIndex = nullptr) const;
        bool IsNewRightThumbstickUp(int controllingPlayer, int* playerIndex) const;
        bool IsNewRightThumbstickDown(int controllingPlayer, int* playerIndex) const;
        bool IsNewRightThumbstickLeft(int controllingPlayer, int* playerIndex) const;
        bool IsNewRightThumbstickRight(int controllingPlayer, int* playerIndex) const;

#ifdef _XBOX_ONE
        // Get the Xbox Gamepad associated with a DirectXTK GamePad index
        //   index - the index of the gamepad to return, or -1 to get the first connected gamepad
        Windows::Xbox::Input::IGamepad^ GetGamepad(int index) const;

        // Get the DirectXTK GamePad index of a Xbox Gamepad (returns -1 if not found)
        int GetGamepadIndex(Windows::Xbox::Input::IGamepad^ gamepad) const;
#endif

        // The current state of all the game pads
        std::array<DirectX::GamePad::State, MaxInputs> CurrentGamePadStates;

        // The previous state of all the game pads
        std::array<DirectX::GamePad::State, MaxInputs> LastGamePadStates;

        std::unique_ptr<DirectX::GamePad> m_gamePad;
        std::unique_ptr<DirectX::Keyboard> m_keyboard;
        std::unique_ptr<DirectX::Mouse> m_mouse;

    private:
        static bool s_isAnyGamepadConnected;

        float m_dpi;
        std::array<DirectX::GamePad::ButtonStateTracker, MaxInputs> m_buttonStateTracker;
        std::unique_ptr<DirectX::Keyboard::KeyboardStateTracker> m_keyboardStateTracker;
        std::unique_ptr<DirectX::Mouse::ButtonStateTracker> m_mouseButtonStateTracker;
        Windows::System::VirtualKey m_lastLetterKeyEvent;
        Windows::System::VirtualKey m_lastNumericKeyEvent;
        wchar_t m_letterPressed;
        uint32_t m_numberPressed;
    };
}
