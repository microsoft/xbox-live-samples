// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "SampleGUI.h"
#include "UserRepeater.h"
#include "LiveResources.h"
//#include "LiveInfoHUD.h"

// A basic sample implementation that creates a D3D11 device and
// provides a render loop.
class Sample : public DX::IDeviceNotify, public std::enable_shared_from_this<Sample>
{
public:

    Sample();

    // Initialization and management
    void Initialize(IUnknown* window, int width, int height, DXGI_MODE_ROTATION rotation);
    void HandleSignin(
        _In_ std::shared_ptr<xbox::services::system::xbox_live_user> user,
        _In_ xbox::services::system::sign_in_status result
        );
    void HandleSignout(_In_ std::shared_ptr<xbox::services::system::xbox_live_user> user);

    // Basic game loop
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
    // SocialManager
    std::vector<std::shared_ptr<xbox::services::social::manager::xbox_social_user_group>> GetSocialGroups();
    // Properties
    void GetDefaultSize( int& width, int& height ) const;

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    void SetupUI();
    void RefreshUserList();

    // SocialManager
    void InitializeSocialManager();
    void InitializeSocialManager(std::shared_ptr<xbox::services::system::xbox_live_user> user);
    void AddUserToSocialManager(_In_ std::shared_ptr<xbox::services::system::xbox_live_user> user);
    void RemoveUserFromSocialManager(_In_ std::shared_ptr<xbox::services::system::xbox_live_user> user);

    void CreateSocialGroupFromList(_In_ std::shared_ptr<xbox::services::system::xbox_live_user> user, _In_ std::vector<string_t> xuidList);
    void DestorySocialGroupFromList(_In_ std::shared_ptr<xbox::services::system::xbox_live_user> user);

    void CreateSocialGroupFromFilters(
        _In_ std::shared_ptr<xbox::services::system::xbox_live_user> user,
        _In_ xbox::services::social::manager::presence_filter presenceDetailLevel,
        _In_ xbox::services::social::manager::relationship_filter filter);

    void DestroySocialGroup(
        _In_ std::shared_ptr<xbox::services::system::xbox_live_user> user,
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
    std::unique_ptr<DirectX::Keyboard>          m_keyboard;
    std::unique_ptr<DirectX::Mouse>             m_mouse;

    DirectX::GamePad::ButtonStateTracker        m_gamePadButtons;
    DirectX::Keyboard::KeyboardStateTracker     m_keyboardButtons;

    // DirectXTK objects.
    //std::unique_ptr<DirectX::GraphicsMemory>    m_graphicsMemory;

    // Xbox Live objects
    std::shared_ptr<ATG::LiveResources>         m_liveResources;
    std::shared_ptr<xbox::services::social::manager::social_manager> m_socialManager;
    function_context m_signInContext;
    function_context m_signOutContext;
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
