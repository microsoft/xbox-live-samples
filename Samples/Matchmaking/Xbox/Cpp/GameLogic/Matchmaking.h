//--------------------------------------------------------------------------------------
// Matchmaking.h
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include "..\Common\DeviceResources.h"
#include "..\Common\StepTimer.h"
#include "SampleGUI.h"
#include "..\LiveResources.h"
#include "..\Utils\SessionListView.h"

// ----------------------------------------------------------------------------
// Defines
// ----------------------------------------------------------------------------
#define TITLE_ID                            0x169056CF
#define GAME_SERVICE_CONFIG_ID              L"097d0100-e05c-4d37-8420-46f1169056cf"
#define LOBBY_TEMPLATE_NAME                 L"LobbySession"
#define PLAYER_SKILL_HOPPER                 L"PlayerSkill"
#define PLAYER_SKILL_NO_QOS_HOPPER          L"PlayerSkillNoQoS"

// QoS Defines
typedef std::chrono::duration<long long, std::ratio<1, 10000000>> timeSpanTicks;
#define TICKS_PER_MILLISECOND               10000
#define TIMEOUT_IN_MILLISECONDS             10000   // Sets the maximum amount of time, in milliseconds, that the system waits to hear back from all remote hosts.
#define NUMBER_OF_PROBES                    8       // Number of desired probe replies to receive. The recommended value for numberOfProbes is 8

// Enable this for capturing performance counters
#define PERF_COUNTERS                       0

enum APPSTATE
{
    APP_MAIN_MENU,
    APP_JOIN_LOBBY,
    APP_JOINING_LOBBY,
    APP_IN_GAME,
};

// list of game modes
static WCHAR* g_gameModeStrings[] =
{
    L"Deathmatch",
    L"Team Battle",
    L"Capture the Flag",
    L"Heist",
    L"Conquest",
    L"Team Assault"
};

// list of maps
static WCHAR* g_mapStrings[] =
{
    L"Kunar Base",
    L"Leyte Gulf",
    L"Ardennes",
    L"Normandy",
    L"Helmand Valley"
};

// A basic sample implementation that creates a D3D11 device and
// provides a render loop.
class Sample : public std::enable_shared_from_this<Sample>
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
    
private:

    // Multiplayer Manager logic
    uint64_t m_multiplayerContext;
    Concurrency::critical_section m_stateLock;
    std::vector<xbox::services::multiplayer::manager::multiplayer_event> m_multiplayerEventQueue;
    std::shared_ptr<xbox::services::multiplayer::manager::multiplayer_manager> m_multiplayerManager;
    Windows::ApplicationModel::Activation::IProtocolActivatedEventArgs^ m_protocolActivatedEventArgs;

    // Multiplayer Manager Methods
    void InitializeMultiplayerManager(_In_ const string_t& templateName = LOBBY_TEMPLATE_NAME);
    void DoWork();
    void FindMatch();
    void CancelMatch();
    void AddLocalUser();
    void RemoveLocalUser();
    void UpdateLocalMemberProperties();
    void UpdateLobbyProperties();
    void SetLobbyHost();
    void UpdateJoinability();
    void UpdateGameProperties();
    void SetGameHost();
    void LeaveGameSession();
    void InviteFriends();

    // QoS
    std::map<string_t, string_t> m_addressToDeviceTokenMap;
    pplx::task<Windows::Xbox::Networking::MeasureQualityOfServiceResult^> measure_qos();
    void perform_qos_measurements();

    void SetupUI();
    void Update(DX::StepTimer const& timer);
    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

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

    // Xbox Live objects
    std::unique_ptr<ATG::LiveResources>         m_liveResources;

    std::unique_ptr<ATG::UIManager>         m_ui;
    std::shared_ptr<DX::TextConsole>       m_eventQueueConsole;
    std::shared_ptr<DX::TextConsole>       m_errorMsgConsole;
    std::shared_ptr<DX::TextConsole>       m_perfConsole;
    ATG::IPanel*                           m_hudPanel;
    ATG::IPanel*                           m_mainMenuPanel;
    ATG::IPanel*                           m_sessionDetailsPanel;
    std::vector< std::wstring >            m_displayEventQueue;
    std::shared_ptr<SessionView>           m_lobbyView;
    std::shared_ptr<SessionView>           m_gameView;
    ATG::TextLabel*                        m_joinabilityLabel;

    // Sample utility helpers 
    APPSTATE m_appState;
    UINT m_mapIndex;
    UINT m_gameModeIndex;
    bool m_isLeavingGame;
    bool m_isFindMatchInProgress;
    float m_clearErrorMsgTimer;

    void ChangeAppStates();
    string_t CreateGuid();
    int32_t GetRandomizedGameModeIndex();
    int32_t GetRandomizedMapIndex();
    void RenderMultiplayerEventQueue();
    void RenderPerfCounters();
    void LogErrorFormat(const wchar_t* strMsg, ...);
    xbox::services::multiplayer::manager::joinability GetRandJoinabilityValue();
    int32_t GetActiveMemberCount(std::vector<std::shared_ptr<xbox::services::multiplayer::manager::multiplayer_member>> members);
    void RenderMultiplayerSessionDetails(
        string_t sessionName,
        std::shared_ptr<xbox::services::multiplayer::manager::multiplayer_member> hostMember,
        std::vector<std::shared_ptr<xbox::services::multiplayer::manager::multiplayer_member>> members,
        web::json::value properties,
        xbox::services::multiplayer::manager::multiplayer_session_type sessionType,
        std::shared_ptr<SessionView> sessionView
    );
};
