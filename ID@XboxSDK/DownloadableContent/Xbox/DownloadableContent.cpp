//--------------------------------------------------------------------------------------
// DownloadableContent.cpp
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "DownloadableContent.h"
#include "UIConstants.h"

#include <collection.h>
#include "ATGColors.h"
#include "WICTextureLoader.h"

extern void ExitSample();

using namespace DirectX;

using namespace Windows::Xbox::Management::Deployment;
using Microsoft::WRL::ComPtr;

namespace
{
    const std::wstring c_productID = L"eabe6ccb-48f2-4cd2-832c-1d9753bd707b";

    const InstalledPackagesFilter c_filterOptions[] =
    {
        InstalledPackagesFilter::AllDownloadableContentOnly, 
        InstalledPackagesFilter::AllRelatedPackages,
        InstalledPackagesFilter::CurrentTitleOnly, 
        InstalledPackagesFilter::RelatedTitlesOnly
    };
}

Sample::Sample() :
    m_frame(0)
{
    // Renders only 2D, so no need for a depth buffer.
    m_deviceResources = std::make_unique<DX::DeviceResources>(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_UNKNOWN);

    m_liveResources = std::make_shared<ATG::LiveResources>();
    m_liveInfoHUD = std::make_unique<ATG::LiveInfoHUD>(L"DownloadableContent Sample");

    ATG::UIConfig config;
    m_ui = std::make_shared<ATG::UIManager>(config);

    m_dlcPackageManager = ref new DownloadableContentPackageManager();

    // Saving this handler as it will get applied to each DLC package that gets mounted.
    m_licenseTerminatedHandler = ref new DLCLicenseTerminatedEventHandler(
        [this](IDownloadableContentPackage^ package, ILicenseTerminatedEventArgs^ args)
    {
        if (args->UserXuidIfCausedBySignout)
        {
            wchar_t buffer[512] = {};
            swprintf_s(buffer, L"License lost from package due to user %ws signout.\n", package->PackageFullName->Data());
            OutputDebugString(buffer);
        }

        if (package->IsMounted)
        {
            // Unmounting immediately on license terminated is not required and is left to the title 
            // to determine the correct time to unmount the package.For example after the current 
            // round / map is completed, or immediately if desired.It is recommended to notify the 
            // user with in - game UI informing them that access to the DLC has been lost and offer 
            // upsell if they continue to try and use it.
        }

        RefreshInstalledPackages();
    });

    //  Register for when a DLC package is installed
    m_packageInstalledToken = m_dlcPackageManager->DownloadableContentPackageInstallCompleted
        += ref new DownloadableContentPackageInstallCompletedEventHandler(
            [this]()
    {
        wchar_t buffer[512] = {};
        swprintf_s(buffer, L"Package install event received.\n");
        OutputDebugString(buffer);

        RefreshInstalledPackages();
    });

    //  Register for when a DLC package is installed with the event that actually specifies what was installed
    m_packageInstalledWithDetailsToken = m_dlcPackageManager->DownloadableContentPackageInstallCompletedWithDetails
        += ref new Windows::Foundation::EventHandler<IDownloadableContentPackageInstallCompletedEventArgs^>(
            [this](Platform::Object^ obj, IDownloadableContentPackageInstallCompletedEventArgs^ args)
    {
        wchar_t buffer[512] = {};
        swprintf_s(buffer, L"Package install with details event received: %ws installed\n", args->PackageFullName->Data());
        OutputDebugString(buffer);

        RefreshInstalledPackages();
    });
}

// Initialize the Direct3D resources required to run.
void Sample::Initialize(IUnknown* window)
{
    m_gamePad = std::make_unique<GamePad>();

    m_ui->LoadLayout(L".\\Assets\\SampleUI.csv", L".\\Assets");

    m_deviceResources->SetWindow(window);

    m_deviceResources->CreateDeviceResources();  
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    m_liveResources->SetUserChangedCallback([this](ATG::XboxLiveUser user)
    {
        m_liveInfoHUD->SetUser(m_liveResources->GetLiveContext());
        RefreshInstalledPackages();
    });

    m_liveResources->SetUserSignOutCompletedCallback([this](ATG::XboxLiveUser user)
    {
        m_liveInfoHUD->SetUser(m_liveResources->GetLiveContext());
        RefreshInstalledPackages();
    });

    SetupUI();

    m_liveResources->Initialize();
    m_liveInfoHUD->Initialize(m_liveResources->GetLiveContext());

    RefreshInstalledPackages();
}

#pragma region DLC Methods
void Sample::RefreshInstalledPackages()
{
    // This flag will only show the DLC that this title is allowed to enumerate.  See documentation 
    // for what the other flags will return.
    auto packages = m_dlcPackageManager->FindPackages(c_filterOptions[m_twist->CurrentIndex()]);

    std::lock_guard<std::mutex> lock(m_dlcPackageListLock);
    std::vector<IDownloadableContentPackage^> tempDLCList;
    tempDLCList.reserve(packages->Size);

    // Refresh the UI with new packages. If the package is mounted then use that package object because the new package
    // return from FindPackages will not be mounted.
    for (auto package : packages)
    {
        auto mounted = m_dlcMountedPackageList.find(package->ContentId->Data());

        if (mounted != m_dlcMountedPackageList.end())
        {
            tempDLCList.push_back(mounted->second);
        }
        else
        {
            tempDLCList.push_back(package);
        }
    }

    m_listView->UpdateRows(tempDLCList);
}

std::wstring Sample::MountSelectedPackage(IDownloadableContentPackage ^package)
{
    if (package == nullptr || package->IsMounted)
    {
        return L"";
    }

    try
    {
        RegisterPackageEvents(package);

        auto mountPoint = package->Mount();

        if (mountPoint == nullptr || mountPoint->IsEmpty())
        {
            wchar_t buffer[512] = {};
            swprintf_s(buffer, L"Empty mount point from package %ws.\n", package->PackageFullName->Data());
            OutputDebugString(buffer);
        }

        {
            // Keep track of mounted packages so we can unmount them later.
            std::lock_guard<std::mutex> lock(m_dlcPackageListLock);
            m_dlcMountedPackageList.insert(std::make_pair(std::wstring(package->ContentId->Data()), package));
        }

        return mountPoint->Data();
    }
    catch (Platform::Exception ^e)
    {
        wchar_t buffer[512] = {};
        swprintf_s(buffer, L"[%08X] Error while mounting - %ws, check if licensed\n", e->HResult, e->Message->Data());
        OutputDebugString(buffer);
        return L"";
    }
}

void Sample::UnmountSelectedPackage(IDownloadableContentPackage ^package)
{
    if (package == nullptr || !package->IsMounted)
    {
        return;
    }

    try
    {
        UnregisterPackageEvents(package);

        package->Unmount();

        {
            std::lock_guard<std::mutex> lock(m_dlcPackageListLock);
            m_dlcMountedPackageList.erase(package->ContentId->Data());
        }
    }
    catch (Platform::Exception^ e)
    {
        wchar_t buffer[512] = {};
        swprintf_s(buffer, L"[%08X] Error while unmounting - %ws\n", e->HResult, e->Message->Data());
        OutputDebugString(buffer);
    }
}

void Sample::RegisterPackageEvents(IDownloadableContentPackage ^package)
{
    std::lock_guard<std::mutex> lock(m_dlcPackageEventLock);
    auto token = package->LicenseTerminated += m_licenseTerminatedHandler;

    m_dlcPackagesEvents.insert(std::make_pair(std::wstring(package->PackageFullName->Data()), token));
}

void Sample::UnregisterPackageEvents(IDownloadableContentPackage ^package)
{
    std::lock_guard<std::mutex> lock(m_dlcPackageEventLock);
    auto registrationEvent = m_dlcPackagesEvents.find(package->PackageFullName->Data());

    if (registrationEvent != m_dlcPackagesEvents.end())
    {
        package->LicenseTerminated -= registrationEvent->second;

        m_dlcPackagesEvents.erase(package->PackageFullName->Data());
    }
}

bool Sample::IsPackageMounted(Windows::Xbox::Management::Deployment::IDownloadableContentPackage ^package) 
{ 
    return m_dlcMountedPackageList.find(package->ContentId->Data()) != m_dlcMountedPackageList.end(); 
}
#pragma endregion

#pragma region UI Methods
void Sample::SetupUI()
{
    using namespace ATG;
    static std::vector<std::wstring> s_options = { L"AllDownloadableContent", L"AllRelatedPackages", L"CurrentTitleOnly", L"RelatedTitlesOnly" };
    m_twist = std::make_unique<UITwist>(m_ui.get(), c_sampleUIPanel, c_twistElementStart, s_options);

    ListViewConfig config = { c_pageSize, c_item01, c_itemOffset, c_sampleUIPanel };
    m_listView = std::make_unique<ListView<IDownloadableContentPackage ^, DLCListViewRow>>(m_ui, config);
    m_listView->ClearAllRows();
    m_listView->SetSelectedCallback([this](IPanel *, IControl *control)
    {
        auto button = reinterpret_cast<DLCListViewRow*>(control->GetUser());

        if (!button->dlcPackage->IsMounted)
        {
            MountSelectedPackage(button->dlcPackage);
        }
        else
        {
            UnmountSelectedPackage(button->dlcPackage);
        }

        button->Update(button->dlcPackage);
    });

    m_ui->FindPanel<ATG::IPanel>(c_sampleUIPanel)->Show();
}

void DLCListViewRow::Update(IDownloadableContentPackage ^item)
{
    if (item->IsMounted)
    {
        m_productImage->SetImageId(c_mountedImage);
    }
    else
    {
        m_productImage->SetImageId(c_unmountedImage);
    }

    // if a license is not found, exception with 0x87E10BC6 is thrown
    bool isTrial = false;
    bool isLicensed = false;
    try
    {
        isLicensed = item->CheckLicense(&isTrial);
    }
    catch (Platform::Exception^ e)
    {
        wchar_t buffer[512] = {};
        swprintf_s(buffer, L"[%08X] Error while checking license - %s\n", e->HResult, e->Message->Data());
        OutputDebugString(buffer);
    };

	wchar_t buffer[512] = {};
	swprintf_s(buffer, L"Package %s isLicensed %d IsMounted %d\n", item->PackageFullName->Data(), isLicensed, item->IsMounted);
	OutputDebugString(buffer);


    m_productLicensed->SetVisible(isLicensed);
    m_displayName->SetText(item->DisplayName->Data());
    m_productId->SetText(item->ProductId->Data());
    m_contentId->SetText(item->ContentId->Data());

    dlcPackage = item;
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

    #pragma message( __FILE__  ": TODO in Update" )
    // TODO: Add your sample logic here.
    elapsedTime;

    auto pad = m_gamePad->GetState(0);
    if (pad.IsConnected())
    {
        m_gamePadButtons.Update(pad);

        if(m_ui->Update(elapsedTime, pad))
        {
            if (pad.IsViewPressed())
            {
                ExitSample();
            }

            if (m_gamePadButtons.menu == GamePad::ButtonStateTracker::PRESSED)
            {
                Windows::Xbox::UI::SystemUI::ShowAccountPickerAsync(nullptr, Windows::Xbox::UI::AccountPickerOptions::None);
            }
        }
        if (m_gamePadButtons.leftShoulder == GamePad::ButtonStateTracker::PRESSED)
        {
            m_twist->MovePrevious();
            m_listViewPage = 0;
            auto panel = m_ui->FindPanel<ATG::IPanel>(c_sampleUIPanel);
            auto control = panel->Find(c_item01);
            if (control->IsVisible())
            {
                panel->SetFocus(control);
            }
        }
        else if (m_gamePadButtons.rightShoulder == GamePad::ButtonStateTracker::PRESSED)
        {
            m_twist->MoveNext();
            m_listViewPage = 0;
            auto panel = m_ui->FindPanel<ATG::IPanel>(c_sampleUIPanel);
            auto control = panel->Find(c_item01);
            if (control->IsVisible())
            {
                panel->SetFocus(control);
            }
        }
        else if (m_gamePadButtons.x == GamePad::ButtonStateTracker::PRESSED)
        {
            // Show the products in the marketplace
            auto op = Windows::Xbox::ApplicationModel::Store::Product::ShowMarketplaceAsync(m_liveResources->GetUser(),
                                                                                            Windows::Xbox::ApplicationModel::Store::ProductItemTypes::Game,
                                                                                            ref new Platform::String(c_productID.c_str()),
                                                                                            Windows::Xbox::ApplicationModel::Store::ProductItemTypes::Durable);

            concurrency::create_task(op).then([this](concurrency::task<void> t)
            {
                try
                {
                    t.get(); // used to catch exceptions from the AsyncOp
                }
                catch (Platform::Exception^ e)
                {
                    OutputDebugStringW(e->ToString()->Data());
                }
            });
        }
		else if (m_gamePadButtons.y == GamePad::ButtonStateTracker::PRESSED)
		{
			RefreshInstalledPackages();
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
        
    m_liveInfoHUD->Render();
    m_ui->Render();

    #pragma message( __FILE__  ": TODO in Render" )
    // TODO: Add your rendering code here.

    PIXEndEvent(context);

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
    auto renderTarget = m_deviceResources->GetRenderTargetView();

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
    m_liveResources->Refresh();
    m_ui->Reset();

    RefreshInstalledPackages();

	OutputDebugString(m_packageInstalledWithDetailsToken.ToString()->Data());
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Sample::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();

    m_graphicsMemory = std::make_unique<GraphicsMemory>(device, m_deviceResources->GetBackBufferCount());

    m_liveInfoHUD->RestoreDevice(context);
    m_ui->RestoreDevice(context);

    ComPtr<ID3D11ShaderResourceView> texture;
    CreateWICTextureFromFile(device, L"\\Assets\\Mounted.png", nullptr, texture.ReleaseAndGetAddressOf());
    m_ui->RegisterImage(c_mountedImage, texture.Get());

    CreateWICTextureFromFile(device, L"\\Assets\\Unmounted.png", nullptr, texture.ReleaseAndGetAddressOf());
    m_ui->RegisterImage(c_unmountedImage, texture.Get());
}

// Allocate all memory resources that change on a window SizeChanged event.
void Sample::CreateWindowSizeDependentResources()
{
    RECT fullscreen = m_deviceResources->GetOutputSize();

    m_ui->SetWindow(fullscreen);
}
#pragma endregion

#pragma region ListViewRow Methods
std::weak_ptr<ATG::UIManager> DLCListViewRow::s_ui;
Sample* DLCListViewRow::s_sample;

void DLCListViewRow::Show()
{
    m_selectBtn->SetVisible(true);
    m_productImage->SetVisible(true);
    m_productLicensed->SetVisible(true);
    m_displayName->SetVisible(true);
    m_pid->SetVisible(true);
    m_productId->SetVisible(true);
    m_cid->SetVisible(true);
    m_contentId->SetVisible(true);
}

void DLCListViewRow::Hide()
{
    m_selectBtn->SetVisible(false);
    m_productImage->SetVisible(false);
    m_productLicensed->SetVisible(false);
    m_displayName->SetVisible(false);
    m_pid->SetVisible(false);
    m_productId->SetVisible(false);
    m_cid->SetVisible(false);
    m_contentId->SetVisible(false);
}

void DLCListViewRow::SetControls(ATG::IPanel *parent, int rowStart)
{
    m_selectBtn = dynamic_cast<ATG::Button*>(parent->Find(rowStart));
    m_selectBtn->SetUser(reinterpret_cast<void*>(this));

    m_productImage = dynamic_cast<ATG::Image*>(parent->Find(rowStart + 1));
    m_productLicensed = dynamic_cast<ATG::Image*>(parent->Find(rowStart + 2));
    m_displayName = dynamic_cast<ATG::TextLabel*>(parent->Find(rowStart + 3));
    m_pid = dynamic_cast<ATG::TextLabel*>(parent->Find(rowStart + 4));
    m_productId = dynamic_cast<ATG::TextLabel*>(parent->Find(rowStart + 5));
    m_cid = dynamic_cast<ATG::TextLabel*>(parent->Find(rowStart + 6));
    m_contentId = dynamic_cast<ATG::TextLabel*>(parent->Find(rowStart + 7));
}

void DLCListViewRow::SetSelectedCallback(ATG::IControl::callback_t callback)
{
    m_selectBtn->SetCallback(callback);
}
#pragma endregion