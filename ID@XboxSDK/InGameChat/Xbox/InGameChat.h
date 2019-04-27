//--------------------------------------------------------------------------------------
// InGameChat.h
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "SampleGUI.h"
#include "UserRepeater.h"
#include "ChatIntegrationLayer.h"
#include "TextConsole.h"

#define SECURE_DEVICE_ASSOCIATION_TEMPLATE_UDP      "PeerToHostUDP"
#define GAME_MULTIPLAYER_SESSION_TEMPLATE_NAME      "LobbySession"

// A basic sample implementation that creates a D3D11 device and
// provides a render loop.
class Sample
{
public:

    Sample();

    // Initialization and management
    void Initialize(IUnknown* window);

    // Basic game loop
    void Tick();
    void Render();

    // Rendering helpers
    void Clear();

    // Messages
    void OnSuspending();
    void OnResuming();
    void OnProtocolActivation(Windows::ApplicationModel::Activation::IProtocolActivatedEventArgs^ args);

    static Sample *Instance() { return s_this; }
    Microsoft::Xbox::Samples::NetworkMesh::MeshManager^ MeshManager() { return m_meshManager; }

    Platform::String^ GetNameFromXuid(Platform::String^ xuid);
    void LogToConsole(Platform::String^ val);

    static const int MAXUSERS = 8;

private:

    void Update(DX::StepTimer const& timer);

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();
    void SetupUI();

    void UpdateControllerUIState();
    void BindControllerToUser(Windows::Xbox::Input::IGamepad^ pad);
    Windows::Xbox::Input::IGamepad^ GamepadFromPadId(uint64_t id);

    void JoinSession();
    void LeaveSession();
    void ProcessMpmMessages();
    void SendTextMessage(Platform::String^ xuid);
    void InviteUsers(Windows::Xbox::System::User^ user);
    void SearchJoinableUsers(Windows::Xbox::System::User^ user);
    void ChangeChannel(Windows::Xbox::System::User^ user, int updown);
    void UpdateUIState();
    void SetupChat();
    void LeaveChat();
    void LoadUserDisplayNames();
    void ReloadDisplayNames();

    void InitializeMeshManager(uint8_t consoleID);
    void RegisterMeshControllerEventHandlers();
    void ShutdownMeshManager();

    std::wstring FormatNumber(double val);


    std::unique_ptr<DX::DeviceResources>            m_deviceResources;
    uint64_t                                        m_frame;
    DX::StepTimer                                   m_timer;
    std::unique_ptr<DirectX::GamePad>               m_gamePad;
    DirectX::GamePad::ButtonStateTracker            m_gamePadButtons;
    std::unique_ptr<DirectX::GraphicsMemory>        m_graphicsMemory;
    std::shared_ptr<ATG::UIManager>                 m_ui;
    std::unique_ptr<UserRepeater>                   m_userList;
    std::unique_ptr<UserRepeater>                   m_chatList;
    std::unique_ptr<UserRepeater>                   m_playerList;
    std::map<Platform::String^, Platform::String^>  m_xuidToHandle;
    std::vector<Platform::String^>                  m_joinableXuids;
    uint64_t                                        m_nextId;
    std::unique_ptr<DX::TextConsole>                m_console;
    bool                                            m_inLobby;
    bool                                            m_selectPlayer;
    bool                                            m_inChat;
    Platform::String^                               m_joinXuid;
    static Sample *                                 s_this;
    concurrency::critical_section                   m_uiLock;
    concurrency::critical_section                   m_stateLock;
    std::map<Platform::String^, Platform::String^>  m_xuidToGamertag;
    float                                           m_refreshTimer;
    bool                                            m_needNames;
    bool                                            m_loadingNames;
    bool                                            m_meshInitialized;
    bool                                            m_leavingToJoin;
    bool                                            m_searching;
    Windows::Foundation::EventRegistrationToken     m_debugToken;
    Windows::Foundation::EventRegistrationToken     m_chatToken;
    Windows::Foundation::EventRegistrationToken     m_handshakeToken;
    Windows::Foundation::EventRegistrationToken     m_disconnectToken;
    Microsoft::Xbox::Services::XboxLiveContext^     m_context;
    bool                                            m_partyActive;
    bool                                            m_partySuppressed;

    Windows::ApplicationModel::Activation::IProtocolActivatedEventArgs^ m_activationArgs;
    Microsoft::Xbox::Samples::NetworkMesh::MeshManager^ m_meshManager;
    Microsoft::Xbox::Services::Multiplayer::Manager::MultiplayerManager^ m_multiplayerManager;
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
