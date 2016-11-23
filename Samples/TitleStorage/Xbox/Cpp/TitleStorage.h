//--------------------------------------------------------------------------------------
// TitleStorage.h
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"

#include "SampleGUI.h"
#include "LiveResources.h"
#include "ListView.h"


// Helper class for displaying the list of files in Title Storage
class TitleStorageViewRow
{
public:
    void Show();
    void Hide();
    void SetControls(ATG::IPanel *parent, int rowStart);
    void Update(const xbox::services::title_storage::title_storage_blob_metadata &item);
    void SetSelectedCallback(ATG::IControl::callback_t callback);
private:
    ATG::Button    *m_selectBtn;
    ATG::TextLabel *m_blobPath;
    ATG::TextLabel *m_blobType;
    ATG::TextLabel *m_displayName;
    ATG::TextLabel *m_length;
    ATG::TextLabel *m_XUID;
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
    void Render();

    // Rendering helpers
    void Clear();

    // Messages
    void OnSuspending();
    void OnResuming();

    // Title Storage
    void ChangeStorageType(xbox::services::title_storage::title_storage_type type);
    void DownloadBlob(xbox::services::title_storage::title_storage_blob_metadata *blob);
    void DeleteBlob(xbox::services::title_storage::title_storage_blob_metadata *blob);
    void UploadBlob(xbox::services::title_storage::title_storage_blob_type type,
                    const std::wstring &blobPath,
                    std::shared_ptr<std::vector<unsigned char>> blobBuffer);

    void ShowNoticePopUp(const wchar_t *noticeType, const wchar_t *message);
    void ShowFileOptions(void *blob, bool canDelete);
private:

    void Update(DX::StepTimer const& timer);

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

    // Xbox Live objects
    std::unique_ptr<ATG::LiveResources>         m_liveResources;

    // UI Objects
    std::unique_ptr<ATG::UIManager>             m_ui;

    std::unique_ptr<ListView<xbox::services::title_storage::title_storage_blob_metadata, TitleStorageViewRow>> m_listView;

    xbox::services::title_storage::title_storage_type                       m_selectedStorageType;
    std::vector<xbox::services::title_storage::title_storage_blob_metadata> m_currentBlobList;
};
