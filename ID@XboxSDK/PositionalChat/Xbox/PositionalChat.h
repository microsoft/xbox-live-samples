//--------------------------------------------------------------------------------------
// PositonalChat.h
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
#include "GameAudio.h"

#define SECURE_DEVICE_ASSOCIATION_TEMPLATE_UDP      "PeerToHostUDP"
#define GAME_MULTIPLAYER_SESSION_TEMPLATE_NAME      "LobbySession"

#define CIRCLE_RADIUS 600

static const float moveScalar = 3.0f;
static const float heightScalarUp = 0.01f;
static const float heightScalarDown = 0.0025f;
static const float maxHeight = 400;

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
    void ReloadDisplayNames();
    void LogToConsole(Platform::String^ val);

    static const int MAXUSERS = 8;

    GameAudio*   m_gameAudio;

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

    void InitializeMeshManager(BYTE consoleID);
    void RegisterMeshControllerEventHandlers();
    void ShutdownMeshManager();

    void ConvertToScreenSpace(float inX, float inY, float* outX, float* outY);
    void DrawCone(float OuterAngle, float InnerAngle, float x, float y, float fAngle, DirectX::XMVECTOR vOutlineColor, UINT size);
    void DrawPointerCross(float x, float y, float fAngle, DirectX::XMVECTOR vOutlineColor);
    void DrawCircle(float x, float y, float radius);
    void DrawGrid();

    std::wstring FormatNumber(double val);

    // Device resources.
    std::unique_ptr<DX::DeviceResources>        m_deviceResources;

    // Rendering loop timer.
    uint64_t                                    m_frame;
    DX::StepTimer                               m_timer;

    // Input devices.
    std::unique_ptr<DirectX::GamePad>           m_gamePad;

    DirectX::GamePad::ButtonStateTracker        m_gamePadButtons;

    // DirectXTK objects.
    std::unique_ptr<DirectX::GraphicsMemory>    m_graphicsMemory;
    std::unique_ptr<DirectX::SpriteFont>        m_font;
    std::unique_ptr<DirectX::CommonStates>      m_states;
    std::unique_ptr<DirectX::SpriteBatch>       m_spriteBatch;
    std::unique_ptr<DirectX::BasicEffect>       m_batchEffect;
    std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_batch;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>   m_batchInputLayout;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_circle;

    // Audio state
    float                   m_fListenerAngle;
    float                   m_fEmitterAngle;
    int                     m_reverbIndex;


    // GUI
    std::shared_ptr<ATG::UIManager>             m_ui;
    std::unique_ptr<UserRepeater>               m_userList;
    std::unique_ptr<UserRepeater>               m_chatList;
    std::unique_ptr<UserRepeater>               m_playerList;
    std::map<Platform::String^, Platform::String^> m_xuidToHandle;
    std::vector<Platform::String^>              m_joinableXuids;
    uint64_t                                    m_nextId;
    std::unique_ptr<DX::TextConsole>            m_console;
    bool                                        m_inLobby;
    bool                                        m_selectPlayer;
    bool                                        m_inChat;
    bool                                        m_adjustPosition;
    Platform::String^                           m_joinXuid;
    static Sample *                             s_this;
    concurrency::critical_section               m_uiLock;
    concurrency::critical_section               m_stateLock;
    std::map<Platform::String^, Platform::String^> m_xuidToGamertag;
    float                                       m_refreshTimer;
    bool                                        m_needNames;
    bool                                        m_loadingNames;
    bool                                        m_meshInitialized;
    bool                                        m_leavingToJoin;
    bool                                        m_searching;
    Windows::Foundation::EventRegistrationToken m_debugToken;
    Windows::Foundation::EventRegistrationToken m_chatToken;
    Windows::Foundation::EventRegistrationToken m_handshakeToken;
    Windows::Foundation::EventRegistrationToken m_disconnectToken;
    Microsoft::Xbox::Services::XboxLiveContext^ m_context;
    bool                                        m_partyActive;
    bool                                        m_partySuppressed;

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
