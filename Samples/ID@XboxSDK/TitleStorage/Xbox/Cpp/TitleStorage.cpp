// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "pch.h"
#include "TitleStorage.h"
#include "ATGColors.h"
#include <codecvt>

using namespace DirectX;

namespace
{
    const int c_sampleUIPanel = 2000;
    const int c_noticePopUpPanel = 7000;
    const int c_fileOptionsPopUpPanel = 8000;
    const int c_uploadPopUpPanel = 9000;

    const int c_globalStorageCheckBox = 2201;
    const int c_jsonStorageCheckBox = 2202;
    const int c_trustedStorageCheckBox = 2203;
    const int c_usedBytesLabel = 2204;
    const int c_quotaBytesLabel = 2205;

    const wchar_t *BlobTypeStrings[] = {
        L"Unknown",
        L"Binary",
        L"JSON",
        L"Config"
    };

    void WrapString(std::wstring &string, size_t lineWidth)
    {
        if(string.find(L'\n') != std::wstring::npos)
            return;

        for (size_t line = lineWidth; line < string.length(); line += lineWidth)
        {
            string.insert(line,L"\n");
        }
    }

    std::shared_ptr<std::vector<unsigned char>> CreateBufferFromString(const std::wstring &string)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
        auto bytes = convert.to_bytes(string);
        return std::make_shared<std::vector<unsigned char>>(std::vector<unsigned char>(bytes.begin(), bytes.end()));
    }

}

Sample::Sample() :
    m_frame(0)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_UNKNOWN);
    m_liveResources = std::make_unique<ATG::LiveResources>();

    ATG::UIConfig uiconfig;
    m_ui = std::make_unique<ATG::UIManager>(uiconfig);
}

// Initialize the Direct3D resources required to run.
void Sample::Initialize(IUnknown* window)
{
    m_gamePad = std::make_unique<GamePad>();

    m_ui->LoadLayout(L".\\Assets\\SampleUI.csv", L".\\Assets");

    m_liveResources->Initialize(m_ui, m_ui->FindPanel<ATG::Overlay>(c_sampleUIPanel));
    m_deviceResources->SetWindow(window);

    m_deviceResources->CreateDeviceResources();  
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    SetupUI();
}

#pragma region UI Methods
void Sample::SetupUI()
{
    using namespace ATG;

    ListViewConfig config = { 10, 3100, 100, c_sampleUIPanel };
    m_listView = std::make_unique<ListView<xbox::services::title_storage::title_storage_blob_metadata, TitleStorageViewRow>>(m_ui, config);
    m_listView->ClearAllRows();

    // When the ListItem is selected, show the File Option Popup
    m_listView->SetSelectedCallback([this](ATG::IPanel *, ATG::IControl *control)
    {
        auto storageMetadata = reinterpret_cast<xbox::services::title_storage::title_storage_blob_metadata*>(control->GetUser());
        ShowFileOptions(storageMetadata, storageMetadata->storage_type() != xbox::services::title_storage::title_storage_type::global_storage);
    });
    // File Option to download selected file
    m_ui->FindControl<ATG::Button>(c_fileOptionsPopUpPanel, 8001)->SetCallback([this](ATG::IPanel *, ATG::IControl *control)
    {
        auto storageMetadata = reinterpret_cast<xbox::services::title_storage::title_storage_blob_metadata*>(control->GetUser());
        DownloadBlob(storageMetadata);
    });
    // File Option to delete selected file
    m_ui->FindControl<ATG::Button>(c_fileOptionsPopUpPanel, 8002)->SetCallback([this](ATG::IPanel *, ATG::IControl *control)
    {
        auto storageMetadata = reinterpret_cast<xbox::services::title_storage::title_storage_blob_metadata*>(control->GetUser());
        DeleteBlob(storageMetadata);
    });

    // Upload button selected
    m_ui->FindControl<ATG::Button>(c_sampleUIPanel, 2206)->SetCallback([this](ATG::IPanel *, ATG::IControl *)
    {
        bool canUploadBinary = xbox::services::title_storage::title_storage_type::trusted_platform_storage == m_selectedStorageType;
        auto button = m_ui->FindControl<ATG::Button>(c_uploadPopUpPanel, 9002);
        button->SetEnabled(canUploadBinary);

        m_ui->FindPanel<ATG::IPanel>(c_uploadPopUpPanel)->Show();
    });
    // Upload the JSON blob
    m_ui->FindControl<ATG::Button>(c_uploadPopUpPanel, 9001)->SetCallback([this](ATG::IPanel *, ATG::IControl *)
    {
        UploadBlob(xbox::services::title_storage::title_storage_blob_type::json,
                   L"SampleData/UserStorageBlob.json",
                   CreateBufferFromString(L"{\n\"isThisJson\": 1,\n\"monstersKilled\": 10,\n\"playerClass\": \"warrior\"\n}"));
    });
    // Upload the binary blob
    m_ui->FindControl<ATG::Button>(c_uploadPopUpPanel, 9002)->SetCallback([this](ATG::IPanel *, ATG::IControl *)
    {
        UploadBlob(xbox::services::title_storage::title_storage_blob_type::binary,
                   L"SampleData/UserStorageBlob.dat",
                   CreateBufferFromString(L"This is a title storage test."));
    });

    // Storage Location options
    m_ui->FindControl<ATG::CheckBox>(c_sampleUIPanel, c_globalStorageCheckBox)->SetCallback([this](ATG::IPanel *parent, ATG::IControl *)
    {
        dynamic_cast<ATG::CheckBox*>(parent->Find(c_globalStorageCheckBox))->SetChecked(true);
        dynamic_cast<ATG::CheckBox*>(parent->Find(c_jsonStorageCheckBox))->SetChecked(false);
        dynamic_cast<ATG::CheckBox*>(parent->Find(c_trustedStorageCheckBox))->SetChecked(false);

        ChangeStorageType(xbox::services::title_storage::title_storage_type::global_storage);
    });
    m_ui->FindControl<ATG::CheckBox>(c_sampleUIPanel, c_jsonStorageCheckBox)->SetCallback([this](ATG::IPanel *parent, ATG::IControl *)
    {
        dynamic_cast<ATG::CheckBox*>(parent->Find(c_globalStorageCheckBox))->SetChecked(false);
        dynamic_cast<ATG::CheckBox*>(parent->Find(c_jsonStorageCheckBox))->SetChecked(true);
        dynamic_cast<ATG::CheckBox*>(parent->Find(c_trustedStorageCheckBox))->SetChecked(false);

        ChangeStorageType(xbox::services::title_storage::title_storage_type::json_storage);
    });
    m_ui->FindControl<ATG::CheckBox>(c_sampleUIPanel, c_trustedStorageCheckBox)->SetCallback([this](ATG::IPanel *parent, ATG::IControl *)
    {
        dynamic_cast<ATG::CheckBox*>(parent->Find(c_globalStorageCheckBox))->SetChecked(false);
        dynamic_cast<ATG::CheckBox*>(parent->Find(c_jsonStorageCheckBox))->SetChecked(false);
        dynamic_cast<ATG::CheckBox*>(parent->Find(c_trustedStorageCheckBox))->SetChecked(true);

        ChangeStorageType(xbox::services::title_storage::title_storage_type::trusted_platform_storage);
    });

    // Default to show Global Storage
    ChangeStorageType(xbox::services::title_storage::title_storage_type::global_storage);
}

void Sample::ShowNoticePopUp(const wchar_t *title, const wchar_t *message)
{
    m_ui->FindControl<ATG::TextLabel>(c_noticePopUpPanel,7001)->SetText(title);
    m_ui->FindControl<ATG::TextLabel>(c_noticePopUpPanel, 7002)->SetText(message);
    m_ui->FindPanel<ATG::IPanel>(c_noticePopUpPanel)->Show();
}

void Sample::ShowFileOptions(void *blob, bool canDelete)
{
    m_ui->FindControl<ATG::Button>(c_fileOptionsPopUpPanel,8001)->SetUser(blob);

    // If the storage location is Global Storage, then we cannot delete the file.
    // Disable the delete option.
    auto deleteBtn = m_ui->FindControl<ATG::Button>(c_fileOptionsPopUpPanel, 8002);
    deleteBtn->SetEnabled(canDelete);
    deleteBtn->SetUser(blob);
    m_ui->FindPanel<ATG::IPanel>(c_fileOptionsPopUpPanel)->Show();
}
#pragma endregion

#pragma region Title Storage
void Sample::ChangeStorageType(xbox::services::title_storage::title_storage_type type)
{
    m_selectedStorageType = type;
    auto &titleStorage = m_liveResources->GetLiveContext()->title_storage_service();

    // Get the Quota for the selected location
    titleStorage.get_quota(m_liveResources->GetServiceConfigId(), type)
    .then([this](xbox::services::xbox_live_result<xbox::services::title_storage::title_storage_quota> result)
    {
        if (result.err())
        {
            std::string msg = result.err_message();
            ShowNoticePopUp(L"Error", std::wstring(msg.begin(), msg.end()).c_str());
            return;
        }

        auto &payload = result.payload();

        m_ui->FindControl<ATG::TextLabel>(c_sampleUIPanel, c_usedBytesLabel)->SetText(std::to_wstring(payload.used_bytes()).c_str());
        m_ui->FindControl<ATG::TextLabel>(c_sampleUIPanel, c_quotaBytesLabel)->SetText(std::to_wstring(payload.quota_bytes()).c_str());
    });    

    // Pass in a XUID for non-global storage
    std::wstring xuid;
    if (type != xbox::services::title_storage::title_storage_type::global_storage)
    {
        xuid = m_liveResources->GetUser()->XboxUserId->Data();
    }

    // Also get list of blobs in the selected location
    titleStorage.get_blob_metadata(m_liveResources->GetServiceConfigId(), type, L"", xuid)
    .then([this](xbox::services::xbox_live_result<xbox::services::title_storage::title_storage_blob_metadata_result> result)
    {
        if(result.err())
        {
            // No need to pop up a message if it's the no blobs for error
            if(result.err().value() != 1008)
            {
                std::string msg = result.err_message();
                ShowNoticePopUp(L"Error", std::wstring(msg.begin(), msg.end()).c_str());
            }
            m_listView->ClearAllRows();
            return;
        }

        auto &payload = result.payload();
        m_currentBlobList = payload.items();

        // Calls into TitleStorageRow::Update
        m_listView->UpdateRows(m_currentBlobList);
    });

    m_ui->FindControl<ATG::Button>(c_sampleUIPanel, 2206)->SetEnabled(type != xbox::services::title_storage::title_storage_type::global_storage);
}

// Pulls data out of the blob metadata for display
void TitleStorageViewRow::Update(const xbox::services::title_storage::title_storage_blob_metadata &item)
{
    // Extract information from the blob metadata for display
    m_blobPath->SetText(item.blob_path().c_str());
    m_blobType->SetText(BlobTypeStrings[static_cast<int>(item.blob_type())]);
    m_displayName->SetText(item.display_name().c_str());
    m_length->SetText(std::to_wstring(item.length()).c_str());
    m_XUID->SetText(item.xbox_user_id().c_str());

    // Save the current blob for easier access then selection action is triggered
    m_selectBtn->SetUser(reinterpret_cast<void*>(&const_cast<xbox::services::title_storage::title_storage_blob_metadata&>(item)));
}

// Downloads the specified blob in the selected storage type.
void Sample::DownloadBlob(xbox::services::title_storage::title_storage_blob_metadata *blob)
{
    auto titleStorage = m_liveResources->GetLiveContext()->title_storage_service();

    // Buffer to download the blob into.
    std::shared_ptr<std::vector<unsigned char>> buffer = std::make_shared<std::vector<unsigned char>>();
    buffer->resize(blob->length());

    // Download the blob and display the contents is a popup
    titleStorage.download_blob(*blob, buffer, xbox::services::title_storage::title_storage_e_tag_match_condition::not_used)
        .then([this](xbox::services::xbox_live_result<xbox::services::title_storage::title_storage_blob_result> result)
    {
        if (result.err())
        {
            std::string msg = result.err_message();
            ShowNoticePopUp(L"Error", std::wstring(msg.begin(), msg.end()).c_str());
            return;
        }

        auto payload = result.payload();
        auto buffer = payload.blob_buffer();

        std::wstring resultString(buffer->begin(), buffer->end());
        WrapString(resultString, 56);
        std::wstring noticeMessage = L"Downloaded Blob Contents:\n";
        noticeMessage.append(resultString);
        // Show popup with file contents
        ShowNoticePopUp(L"Download Result", noticeMessage.c_str());
    });
}

// Deletes the specified blob in the selected storage type.
void Sample::DeleteBlob(xbox::services::title_storage::title_storage_blob_metadata *blob)
{
    // Delete the selected blob
    auto titleStorage = m_liveResources->GetLiveContext()->title_storage_service();
    titleStorage.delete_blob(*blob,false)
        .then([this, blob](xbox::services::xbox_live_result<void> result)
        {
            if (result.err())
            {
                std::string msg = result.err_message();
                ShowNoticePopUp(L"Error", std::wstring(msg.begin(), msg.end()).c_str());
                return;
            }
            ChangeStorageType(m_selectedStorageType);
        });
}

// Uploads a blob to a specific path in the selected storage type.
void Sample::UploadBlob(xbox::services::title_storage::title_storage_blob_type blobType,
                        const std::wstring &blobPath,
                        std::shared_ptr<std::vector<unsigned char>> blobBuffer)
{
    
    xbox::services::title_storage::title_storage_blob_metadata blob(m_liveResources->GetServiceConfigId(),
                                                                    m_selectedStorageType,
                                                                    blobPath,
                                                                    blobType,
                                                                    m_liveResources->GetUserXUID());

    auto titleStorage = m_liveResources->GetLiveContext()->title_storage_service();
    titleStorage.upload_blob(blob,blobBuffer,xbox::services::title_storage::title_storage_e_tag_match_condition::not_used)
        .then([this](xbox::services::xbox_live_result<xbox::services::title_storage::title_storage_blob_metadata> result)
        {
            if (result.err())
            {
                std::string msg = result.err_message();
                ShowNoticePopUp(L"Error", std::wstring(msg.begin(), msg.end()).c_str());
                return;
            }
            ChangeStorageType(m_selectedStorageType);
        });
}
#pragma endregion

#pragma region Frame Update
// Executes basic render loop.
void Sample::Tick()
{
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Frame %I64u", m_frame);

    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();

    PIXEndEvent();
    m_frame++;
}

// Updates the world.
void Sample::Update(DX::StepTimer const& timer)
{
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update");

    float elapsedTime = float(timer.GetElapsedSeconds());

    auto pad = m_gamePad->GetState(0);
    if (pad.IsConnected())
    {
        m_gamePadButtons.Update(pad);

        if (!m_ui->Update(elapsedTime, pad))
        {
            if (pad.IsViewPressed())
            {
                Windows::ApplicationModel::Core::CoreApplication::Exit();
            }
            if (pad.IsMenuPressed())
            {
                Windows::Xbox::UI::SystemUI::ShowAccountPickerAsync(nullptr, Windows::Xbox::UI::AccountPickerOptions::None);
            }
        }
    }
    else
    {
        m_gamePadButtons.Reset();
    }

    PIXEndEvent();
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Sample::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    // Prepare the render target to render a new frame.
    m_deviceResources->Prepare();
    Clear();

    auto context = m_deviceResources->GetD3DDeviceContext();
    PIXBeginEvent(context, PIX_COLOR_DEFAULT, L"Render");

    PIXEndEvent(context);

    m_ui->Render();

    // Show the new frame.
    PIXBeginEvent(context, PIX_COLOR_DEFAULT, L"Present");
    m_deviceResources->Present();
    m_graphicsMemory->Commit();
    PIXEndEvent(context);
}

// Helper method to clear the back buffers.
void Sample::Clear()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    PIXBeginEvent(context, PIX_COLOR_DEFAULT, L"Clear");

    // Clear the views.
    auto renderTarget = m_deviceResources->GetBackBufferRenderTargetView();

    context->ClearRenderTargetView(renderTarget, ATG::Colors::Background);

    context->OMSetRenderTargets(1, &renderTarget, nullptr);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    PIXEndEvent(context);
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Sample::OnSuspending()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    context->Suspend(0);
}

void Sample::OnResuming()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    context->Resume();
    m_timer.ResetElapsedTime();
    m_gamePadButtons.Reset();
    m_ui->Reset();
    m_liveResources->Refresh();
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Sample::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();

    m_graphicsMemory = std::make_unique<GraphicsMemory>(device, m_deviceResources->GetBackBufferCount());

    m_ui->RestoreDevice(context);
}

// Allocate all memory resources that change on a window SizeChanged event.
void Sample::CreateWindowSizeDependentResources()
{
    RECT fullscreen = m_deviceResources->GetOutputSize();
    static const RECT consoleDisplay = { 960, 150, 1838, 950 };

    m_ui->SetWindow(fullscreen);
}
#pragma endregion

#pragma region ListViewRow
void TitleStorageViewRow::Show()
{
    m_selectBtn->SetVisible(true);
    m_selectBtn->SetEnabled(true);
    m_blobPath->SetVisible(true);
    m_blobType->SetVisible(true);
    m_displayName->SetVisible(true);
    m_length->SetVisible(true);
    m_XUID->SetVisible(true);
}
void TitleStorageViewRow::Hide()
{
    m_selectBtn->SetVisible(false);
    m_selectBtn->SetEnabled(false);
    m_blobPath->SetVisible(false);
    m_blobType->SetVisible(false);
    m_displayName->SetVisible(false);
    m_length->SetVisible(false);
    m_XUID->SetVisible(false);
}

void TitleStorageViewRow::SetControls(ATG::IPanel *parent, int rowStart)
{
    m_selectBtn = dynamic_cast<ATG::Button*>(parent->Find(rowStart));
    m_blobPath = dynamic_cast<ATG::TextLabel*>(parent->Find(rowStart + 1));
    m_blobType = dynamic_cast<ATG::TextLabel*>(parent->Find(rowStart + 2));
    m_displayName = dynamic_cast<ATG::TextLabel*>(parent->Find(rowStart + 3));
    m_length = dynamic_cast<ATG::TextLabel*>(parent->Find(rowStart + 4));
    m_XUID = dynamic_cast<ATG::TextLabel*>(parent->Find(rowStart + 5));
}

void TitleStorageViewRow::SetSelectedCallback(ATG::IControl::callback_t callback)
{
    m_selectBtn->SetCallback(callback);
}
#pragma endregion