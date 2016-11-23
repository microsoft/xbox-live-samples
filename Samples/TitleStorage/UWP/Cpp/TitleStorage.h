//--------------------------------------------------------------------------------------
// TitleStorage.h
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include "SampleGUI.h"
#include "DeviceResources.h"
#include "LiveResources.h"
#include "StepTimer.h"
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
class Sample : public DX::IDeviceNotify
{
public:

    Sample();

    // Initialization and management
    void Initialize(IUnknown* window, int width, int height, DXGI_MODE_ROTATION rotation);

    // Basic render loop
    void Tick();
    void Render();

    // Rendering helpers
    void Clear();

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

    // Properties
    void GetDefaultSize( int& width, int& height ) const;

private:

    // Title Storage
    void ChangeStorageType(xbox::services::title_storage::title_storage_type type);
    void DownloadBlob(xbox::services::title_storage::title_storage_blob_metadata *blob);
    void DeleteBlob(xbox::services::title_storage::title_storage_blob_metadata *blob);
    void UploadBlob(xbox::services::title_storage::title_storage_blob_type type,
        const std::wstring &blobPath,
        std::shared_ptr<std::vector<unsigned char>> blobBuffer);

    void ShowNoticePopUp(const wchar_t *noticeType, const wchar_t *message);
    void ShowFileOptions(void *blob, bool canDelete);

    void SetupUI();
    void Update(DX::StepTimer const& timer);
    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;

    // Input devices.
    std::unique_ptr<DirectX::GamePad>       m_gamePad;
    std::unique_ptr<DirectX::Keyboard>      m_keyboard;
    std::unique_ptr<DirectX::Mouse>         m_mouse;

    // UI
    std::unique_ptr<ATG::UIManager>         m_ui;
    std::unique_ptr<DX::TextConsole>       m_console;

    // Xbox Live objects
    std::unique_ptr<ATG::LiveResources>     m_liveResources;

    DirectX::GamePad::ButtonStateTracker    m_gamePadButtons;
    DirectX::Keyboard::KeyboardStateTracker m_keyboardButtons;

    std::unique_ptr<ListView<xbox::services::title_storage::title_storage_blob_metadata, TitleStorageViewRow>> m_listView;
    xbox::services::title_storage::title_storage_type                       m_selectedStorageType;
    std::vector<xbox::services::title_storage::title_storage_blob_metadata> m_currentBlobList;
};