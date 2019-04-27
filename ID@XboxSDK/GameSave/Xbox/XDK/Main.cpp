//--------------------------------------------------------------------------------------
// Main.cpp
//
// Entry point for Xbox One exclusive title.
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "SampleGame.h"
#include "Telemetry.h"

using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::Foundation;
using namespace DirectX;

ref class ViewProvider sealed : public IFrameworkView
{
public:
    ViewProvider() :
        m_exit(false)
    {
    }

    // IFrameworkView methods
    virtual void Initialize(CoreApplicationView^ applicationView)
    {
        applicationView->Activated += ref new
            TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &ViewProvider::OnActivated);

        CoreApplication::Suspending +=
            ref new EventHandler<SuspendingEventArgs^>(this, &ViewProvider::OnSuspending);

        CoreApplication::Resuming +=
            ref new EventHandler<Platform::Object^>(this, &ViewProvider::OnResuming);

        m_sample = std::make_unique<GameSaveSample::SampleGame>();

        // Sample Usage Telemetry
        //
        // Disable or remove this code block to opt-out of sample usage telemetry
        //
        if (EventRegisterATGSampleTelemetry() == ERROR_SUCCESS)
        {
            wchar_t exeName[MAX_PATH + 1] = {};
            if (!GetModuleFileNameW(nullptr, exeName, MAX_PATH))
            {
                wcscpy_s(exeName, L"Unknown");
            }
            wchar_t fname[_MAX_FNAME] = {};
            wchar_t ext[_MAX_EXT] = {};
            (void)_wsplitpath_s(exeName, nullptr, 0, nullptr, 0, fname, _MAX_FNAME, ext, _MAX_EXT);
            (void)_wmakepath_s(exeName, nullptr, nullptr, fname, ext); // keep only the filename + extension

            EventWriteSampleLoaded(exeName);
        }
    }

    virtual void Uninitialize()
    {
        m_sample.reset();
    }

    virtual void SetWindow(CoreWindow^ window)
    {
        window->Closed +=
            ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &ViewProvider::OnWindowClosed);

        // Default window thread to CPU 0
        SetThreadAffinityMask(GetCurrentThread(), 0x1);

        m_sample->Initialize(reinterpret_cast<IUnknown*>(window));
    }

    virtual void Load(Platform::String^ entryPoint)
    {
    }

    virtual void Run()
    {
        while (!m_exit)
        {
            m_sample->Tick();

            CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
        }
    }

protected:
    // Event handlers
    void OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
    {
        Log::Write("OnActivated(%ws)\n", args->Kind.ToString()->Data());

        CoreWindow::GetForCurrentThread()->Activate();
    }

    void OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
    {
        auto deferral = args->SuspendingOperation->GetDeferral();

        create_task([this, deferral]()
        {
            m_sample->OnSuspending(deferral); // main is responsible for calling Complete on the deferral
        });
    }

    void OnResuming(Platform::Object^ sender, Platform::Object^ args)
    {
        m_sample->OnResuming();
    }

    void OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
    {
        m_exit = true;
    }

private:
    bool                                            m_exit;
    std::unique_ptr<GameSaveSample::SampleGame>     m_sample;
};

ref class ViewProviderFactory : IFrameworkViewSource
{
public:
    virtual IFrameworkView^ CreateView()
    {
        return ref new ViewProvider();
    }
};


// Entry point
[Platform::MTAThread]
int __cdecl main(Platform::Array<Platform::String^>^ /*argv*/)
{
    Log::Initialize();

    // Default main thread to CPU 0
    SetThreadAffinityMask(GetCurrentThread(), 0x1);

    auto viewProviderFactory = ref new ViewProviderFactory();
    CoreApplication::Run(viewProviderFactory);
    return 0;
}


// Exit helper
void ExitSample()
{
    Windows::ApplicationModel::Core::CoreApplication::Exit();
}
