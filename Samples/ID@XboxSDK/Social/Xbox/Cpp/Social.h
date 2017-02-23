// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "SampleGUI.h"
#include "UserRepeater.h"
#include "LiveResourcesXDK.h"
#include "LiveInfoHUD.h"

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

    // Messages
    void OnSuspending();
    void OnResuming();

    // SocialManager
    std::vector<std::shared_ptr<xbox::services::social::manager::xbox_social_user_group>> GetSocialGroups();

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    void SetupUI();
    void RefreshUserList();

    // SocialManager
    void InitializeSocialManager(Windows::Foundation::Collections::IVectorView<Windows::Xbox::System::User^>^ userList);
    void InitializeSocialManager(Windows::Xbox::System::User^ user);
    void AddUserToSocialManager(_In_ Windows::Xbox::System::User^ user);
    void RemoveUserFromSocialManager(_In_ Windows::Xbox::System::User^ user);

    void CreateSocialGroupFromList(_In_ Windows::Xbox::System::User^ user, _In_ std::vector<string_t> xuidList);
    void DestorySocialGroupFromList(_In_ Windows::Xbox::System::User^ user);

    void CreateSocialGroupFromFilters(
        _In_ Windows::Xbox::System::User^ user,
        _In_ xbox::services::social::manager::presence_filter presenceDetailLevel,
        _In_ xbox::services::social::manager::relationship_filter filter);

    void DestroySocialGroup(
        _In_ Windows::Xbox::System::User^ user,
        _In_ xbox::services::social::manager::presence_filter presenceDetailLevel,
        _In_ xbox::services::social::manager::relationship_filter filter);

    void UpdateSocialManager();

    enum friendListType
    {
        allFriends,
        allOnlineFriends,
        allOnlineInTitleFriends,
        allFavorites,
        custom
    };

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
    std::shared_ptr<ATG::LiveResources>         m_liveResources;
    std::unique_ptr<ATG::LiveInfoHUD>           m_liveInfoHud;
    std::shared_ptr<xbox::services::social::manager::social_manager> m_socialManager;
    std::mutex m_socialManagerLock;
    std::vector<string_t> m_xuidsInCustomSocialGroup;
    std::vector<std::shared_ptr<xbox::services::social::manager::xbox_social_user_group>> m_socialGroups;

    // UI Objects
    std::shared_ptr<ATG::UIManager>             m_ui;
    std::unique_ptr<DX::TextConsole>            m_console;
    std::unique_ptr<UserRepeater>               m_userList;

    // UI State
    friendListType                              m_selectedFriendList;
};
