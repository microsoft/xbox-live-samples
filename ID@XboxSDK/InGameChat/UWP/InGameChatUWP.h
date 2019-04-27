//--------------------------------------------------------------------------------------
// InGameChatUWP.h
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "SampleGUI.h"
#include "ChatIntegrationLayer.h"
#include "UserRepeater.h"
#include "TextConsole.h"

#define SECURE_DEVICE_ASSOCIATION_TEMPLATE_UDP      L"PeerToHostUDP"
#define GAME_MULTIPLAYER_SESSION_TEMPLATE_NAME      L"LobbySession"

namespace InGameChat
{
// A basic sample implementation that creates a D3D11 device and
// provides a render loop.
class Sample : public DX::IDeviceNotify
{
public:

    Sample();

    // Initialization and management
    void Initialize(IUnknown* window, int width, int height, DXGI_MODE_ROTATION rotation);

    // Basic render loop
    void Tick();

    // IDeviceNotify
    virtual void OnDeviceLost() override;
    virtual void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowSizeChanged(int width, int height, DXGI_MODE_ROTATION rotation);
    void ValidateDevice();
    void OnProtocolActivation(Windows::ApplicationModel::Activation::IProtocolActivatedEventArgs^ args);

    // Properties
    void GetDefaultSize(int& width, int& height) const;

    static Sample *Instance() { return s_this; }
    Microsoft::Xbox::Samples::NetworkMesh::MeshManager^ MeshManager() { return m_meshManager; }

    std::shared_ptr<xbox::services::system::xbox_live_user> GetCurrentUser() { return m_user; }
    std::wstring GetNameFromXuid(std::wstring xuid);
    void ReloadDisplayNames();
    void LogToConsole(Platform::String^ val);

    static const int MAXUSERS = 8;

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    void SetupUI();

    void UpdateControllerUIState();
    void SignInUser();

    void JoinSession();
    void LeaveSession();
    void ProcessMpmMessages();
    void SendTextMessage(Platform::String^ message);
    void InviteUsers();
    void SearchJoinableUsers();
    void ChangeChannel(int updown);
    void SetupChat();
    void LeaveChat();
    void LoadUserDisplayNames();

    void InitializeMeshManager(BYTE consoleID);
    void RegisterMeshControllerEventHandlers();
    void ShutdownMeshManager();

    std::wstring FormatNumber(double val);

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;

    // Input devices.
    std::unique_ptr<DirectX::GamePad>       m_gamePad;
    std::unique_ptr<DirectX::Keyboard>      m_keyboard;
    std::unique_ptr<DirectX::Mouse>         m_mouse;

    DirectX::GamePad::ButtonStateTracker    m_gamePadButtons;
    DirectX::Keyboard::KeyboardStateTracker m_keyboardButtons;

    // GUI
    std::shared_ptr<ATG::UIManager>             m_ui;
    std::unique_ptr<UserRepeater>               m_userList;
    std::unique_ptr<UserRepeater>               m_chatList;
    std::unique_ptr<UserRepeater>               m_playerList;
    std::map<std::wstring, std::wstring>        m_xuidToHandle;
    std::vector<std::wstring>                   m_joinableXuids;
    uint64_t                                    m_nextId;
    std::unique_ptr<DX::TextConsole>            m_console;
    bool                                        m_inLobby;
    bool                                        m_selectPlayer;
    bool                                        m_viewStats;
    bool                                        m_inChat;
    std::wstring                                m_joinXuid;
    static Sample *                             s_this;
    std::mutex                                  m_uiLock;
    std::mutex                                  m_stateLock;
    std::map<std::wstring, std::wstring>        m_xuidToGamertag;
    float                                       m_refreshTimer;
    bool                                        m_needNames;
    bool                                        m_loadingNames;
    bool                                        m_meshInitialized;
    bool                                        m_leavingToJoin;
    bool                                        m_searching;
    bool                                        m_networkReady;
    Windows::Foundation::EventRegistrationToken m_debugToken;
    Windows::Foundation::EventRegistrationToken m_chatToken;
    Windows::Foundation::EventRegistrationToken m_handshakeToken;
    Windows::Foundation::EventRegistrationToken m_disconnectToken;
    std::shared_ptr<xbox::services::xbox_live_context> m_context;
    std::shared_ptr<xbox::services::system::xbox_live_user> m_user;

    Windows::ApplicationModel::Activation::IProtocolActivatedEventArgs^ m_activationArgs;
    Microsoft::Xbox::Samples::NetworkMesh::MeshManager^ m_meshManager;
    std::shared_ptr<xbox::services::multiplayer::manager::multiplayer_manager> m_multiplayerManager;
};

// Helper for output debug tracing
inline void DebugTrace(_In_z_ _Printf_format_string_ const char* format, ...)
{
#ifdef _DEBUG
    va_list args;
    va_start(args, format);

    char buff[1024] = {};
    vsprintf_s(buff, format, args);
    std::wstring wbuff(buff, buff + 1023);
    Sample::Instance()->LogToConsole(ref new Platform::String(wbuff.c_str()));
    va_end(args);
#else
    UNREFERENCED_PARAMETER(format);
#endif
}

}