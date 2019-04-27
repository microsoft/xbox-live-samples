//--------------------------------------------------------------------------------------
// DownloadableContent.h
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "LiveResourcesXDK.h"
#include "LiveInfoHUD.h"
#include "SampleGUI.h"
#include "UITwist.h"
#include "ListView.h"

typedef Windows::Foundation::TypedEventHandler<Windows::Xbox::Management::Deployment::IDownloadableContentPackage^, 
                                               Windows::Xbox::Management::Deployment::ILicenseTerminatedEventArgs^ > DLCLicenseTerminatedEventHandler;

class Sample;

class DLCListViewRow
{
public:
    void Show();
    void Hide();
    void SetControls(ATG::IPanel *parent, int rowStart);
    void Update(Windows::Xbox::Management::Deployment::IDownloadableContentPackage ^item);
    void SetSelectedCallback(ATG::IControl::callback_t callback);

    Windows::Xbox::Management::Deployment::IDownloadableContentPackage ^dlcPackage;

    static std::weak_ptr<ATG::UIManager> s_ui;
    static Sample*                       s_sample;
private:
    ATG::Button    *m_selectBtn;
    ATG::Image     *m_productImage;
    ATG::Image     *m_productLicensed;
    ATG::TextLabel *m_displayName;
    ATG::TextLabel *m_pid;
    ATG::TextLabel *m_productId;
    ATG::TextLabel *m_cid;
    ATG::TextLabel *m_contentId;
};

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

    // DLC Methods
    void RefreshInstalledPackages();
    std::wstring MountSelectedPackage(Windows::Xbox::Management::Deployment::IDownloadableContentPackage ^package);
    void UnmountSelectedPackage(Windows::Xbox::Management::Deployment::IDownloadableContentPackage ^package);
    void RegisterPackageEvents(Windows::Xbox::Management::Deployment::IDownloadableContentPackage ^package);
    void UnregisterPackageEvents(Windows::Xbox::Management::Deployment::IDownloadableContentPackage ^package);
    bool IsPackageMounted(Windows::Xbox::Management::Deployment::IDownloadableContentPackage ^package);

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    void SetupUI();

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

    // UI objects.
    std::shared_ptr<ATG::UIManager>                                                                                m_ui;
    std::unique_ptr<UITwist>                                                                                       m_twist;
    std::unique_ptr<ListView<Windows::Xbox::Management::Deployment::IDownloadableContentPackage^, DLCListViewRow>> m_listView;
    size_t                                                                                                         m_listViewPage;
    
    // Xbox Live objects.
    std::shared_ptr<ATG::LiveResources>         m_liveResources;
    std::unique_ptr<ATG::LiveInfoHUD>           m_liveInfoHUD;

    // DLC Related Objects
    Windows::Xbox::Management::Deployment::IDownloadableContentPackageManager^                  m_dlcPackageManager;

    std::mutex                                                                                  m_dlcPackageListLock;
    std::map<std::wstring, Windows::Xbox::Management::Deployment::IDownloadableContentPackage^> m_dlcMountedPackageList;
    std::mutex                                                                                  m_dlcPackageEventLock;
    std::map<std::wstring, Windows::Foundation::EventRegistrationToken>                         m_dlcPackagesEvents;

    Windows::Foundation::EventRegistrationToken                                                 m_purchaseEvent;
    Windows::Foundation::EventRegistrationToken                                                 m_packageInstalledToken;
    Windows::Foundation::EventRegistrationToken                                                 m_packageInstalledWithDetailsToken;

    DLCLicenseTerminatedEventHandler^                                                           m_licenseTerminatedHandler;
};
