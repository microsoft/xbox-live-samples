// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "pch.h"
#include "MainPage.xaml.h"
#include "xsapi\services.h"

using namespace concurrency;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace TCUI;

struct ScenarioDescriptionItem
{
    int tag;
    Platform::String^ name;
};

enum ScenarioItemTag
{
    Scenario_GetUserProfileAsync = 1,
    Scenario_PeoplePickerAsyncTask,
    Scenario_SendInviteAsyncTask,
    Scenario_ShowProfileCardUI,
    Scenario_ShowChangeFriendRelationshipUI,
    Scenario_ShowTitleAchievementsUI,
    Scenario_CheckGamingPrivilegeWithUI
};

ScenarioDescriptionItem ScenarioDescriptions[] =
{
    { Scenario_GetUserProfileAsync, L"Get user profile" },
    { Scenario_PeoplePickerAsyncTask, "Show People Picker" },
    { Scenario_SendInviteAsyncTask, "Send Invite" },
    { Scenario_ShowProfileCardUI, "Show Profile Card UI" },
    { Scenario_ShowChangeFriendRelationshipUI, "Show Change Friend Relationship UI" },
    { Scenario_ShowTitleAchievementsUI, "Show Title Achievements UI" },
    //{ Scenario_CheckGamingPrivilegeWithUI, "Check Gaming Privilege With UI" } // Requires custom build of XSAPI for now
};

bool MainPage::RunScenario(int selectedTag)
{
    if (m_xboxLiveContext == nullptr || !m_xboxLiveContext->user()->is_signed_in())
    {
        Log(L"No user signed in");
        return false;
    }

    switch (selectedTag)
    {
        case Scenario_GetUserProfileAsync: m_scenarios.Scenario_GetUserProfileAsync(this, m_xboxLiveContext); break;

        case Scenario_PeoplePickerAsyncTask: m_scenarios.Scenario_PeoplePickerAsyncTask(this, m_xboxLiveContext); break;
        case Scenario_SendInviteAsyncTask: m_scenarios.Scenario_SendInviteAsyncTask(this, m_xboxLiveContext); break;
        case Scenario_ShowProfileCardUI: m_scenarios.Scenario_ShowProfileCardUI(this, m_xboxLiveContext); break;
        case Scenario_ShowChangeFriendRelationshipUI: m_scenarios.Scenario_ShowChangeFriendRelationshipUI(this, m_xboxLiveContext); break;
        case Scenario_ShowTitleAchievementsUI: m_scenarios.Scenario_ShowTitleAchievementsUI(this, m_xboxLiveContext); break;
        case Scenario_CheckGamingPrivilegeWithUI: m_scenarios.Scenario_CheckGamingPrivilegeWithUI(this, m_xboxLiveContext); break;

        default: return false;
    }

    return true;
}

MainPage::MainPage()
{
    InitializeComponent();
    this->CoreDispatcher = Windows::UI::Xaml::Window::Current->CoreWindow->Dispatcher;

    for (ScenarioDescriptionItem scenario : ScenarioDescriptions)
    {
        ListBoxItem^ listBoxItem = ref new ListBoxItem();
        listBoxItem->Content = scenario.name;
        listBoxItem->Tag = (Platform::Object^)scenario.tag;
        this->ScenarioListBox->Items->Append(listBoxItem);
    }

    this->ScenarioListBox->SelectedIndex = 0;
    m_user = std::make_shared< xbox::services::system::xbox_live_user >();
    m_user->add_sign_out_completed_handler([this](const xbox::services::system::sign_out_completed_event_args& args)
    {
        // dispaly log on the UI thread
        Dispatcher->RunAsync(
            Windows::UI::Core::CoreDispatcherPriority::Normal,
            ref new Windows::UI::Core::DispatchedHandler([this, args]()
        {
            this->UserInfoLabel->Text = L"user signed out";

            Log(L"----------------");
            Log(StringFormat(L"User %s signed out", args.user()->gamertag().c_str()));

        }));
    });

    Windows::Networking::Connectivity::NetworkInformation::NetworkStatusChanged +=
        ref new Windows::Networking::Connectivity::NetworkStatusChangedEventHandler([this](Platform::Object^ sender)
    {
        UpdateInternetAccessUI();
    });
    UpdateInternetAccessUI();

    SignIn();
}

void MainPage::ScenarioListBox_DoubleTapped(
    Platform::Object^ sender, 
    Windows::UI::Xaml::Input::DoubleTappedRoutedEventArgs^ e
    )
{
    RunSelectedScenario();
}

void MainPage::ClearLogs()
{
    this->OutputStackPanel->Children->Clear();
}

Platform::String^ StringFormat(LPCWSTR strMsg, ...)
{
    WCHAR strBuffer[2048];

    va_list args;
    va_start(args, strMsg);
    _vsnwprintf_s(strBuffer, 2048, _TRUNCATE, strMsg, args);
    strBuffer[2047] = L'\0';

    va_end(args);

    Platform::String^ str = ref new Platform::String(strBuffer);
    return str;
}

void MainPage::Log(Platform::String^ logLine)
{
    CoreDispatcher->RunAsync(
        Windows::UI::Core::CoreDispatcherPriority::Normal,
        ref new Windows::UI::Core::DispatchedHandler([logLine, this]()
    {
        TextBlock^ uiElement = ref new TextBlock();
        uiElement->FontSize = 14;
        uiElement->Text = logLine;
        this->OutputStackPanel->Children->Append(uiElement);
    }));
}

void MainPage::RunButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    RunSelectedScenario();
}

void MainPage::RunSelectedScenario()
{
    if (this->ScenarioListBox->SelectedItems->Size == 1)
    {
        ClearLogs();
        ListBoxItem^ selectedItem = safe_cast<ListBoxItem^>(this->ScenarioListBox->SelectedItem);
        int selectedTag = safe_cast<int>(selectedItem->Tag);
        RunScenario(selectedTag);
    }
}

void MainPage::RunAllButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    ClearLogs();
    int scenarioTag = 1;
    for (;;)
    {
        bool success = RunScenario(scenarioTag);
        if (!success)
        {
            break;
        }

        scenarioTag++;
    }
}

void MainPage::SignInButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    SignIn();
}

void MainPage::SignInSilently()
{
    this->UserInfoLabel->Text = L"Trying to sign in silently...";
    Log(this->UserInfoLabel->Text);

    m_user->signin_silently()
        .then([this](xbox::services::xbox_live_result<xbox::services::system::sign_in_result> t) // use task_continuation_context::use_current() to make the continuation task running in current apartment 
    {
        if (!t.err())
        {
            auto result = t.payload();
            switch (result.status())
            {
            case xbox::services::system::sign_in_status::success:
                OnSignInSucceeded();
                break;
            case xbox::services::system::sign_in_status::user_interaction_required:
                this->UserInfoLabel->Text = L"user_interaction_required";
                Log(L"user_interaction_required");
                break;
            }
        }
        else
        {
            this->UserInfoLabel->Text = L"Sign in silently failed";
            Log(L"signin_silently failed.");
            string_t utf16Error = utility::conversions::utf8_to_utf16(t.err_message());
            Log(ref new Platform::String(utf16Error.c_str()));
        }
    }, task_continuation_context::use_current());
}

void MainPage::SignIn()
{
    this->UserInfoLabel->Text = L"Trying to sign in...";
    Log(this->UserInfoLabel->Text);

    m_user->signin(this->CoreDispatcher)
        .then([this](xbox::services::xbox_live_result<xbox::services::system::sign_in_result> t) // use task_continuation_context::use_current() to make the continuation task running in current apartment 
    {
        if (!t.err())
        {
            auto result = t.payload();
            switch (result.status())
            {
            case xbox::services::system::sign_in_status::success:
                OnSignInSucceeded();
                break;
            case xbox::services::system::sign_in_status::user_cancel:
                this->UserInfoLabel->Text = L"user_cancel";
                Log(L"user_cancel");
                break;
            }
        }
        else
        {
            this->UserInfoLabel->Text = L"Sign in failed";
            Log(L"SignInAsync failed.");
            string_t utf16Error = utility::conversions::utf8_to_utf16(t.err_message());
            Log(ref new Platform::String(utf16Error.c_str()));
        }

    }, task_continuation_context::use_current());
}


void MainPage::OnSignInSucceeded()
{
    m_xboxLiveContext = std::make_shared< xbox::services::xbox_live_context >(m_user);
    xbox::services::system::xbox_live_services_settings::get_singleton_instance()->set_diagnostics_trace_level(xbox::services::xbox_services_diagnostics_trace_level::verbose);

    m_xboxLiveContext->settings()->add_service_call_routed_handler([this](xbox::services::xbox_service_call_routed_event_args args)
    {
        if (args.http_status() >= 300)
        {
            Log(StringFormat(L"[URL]: %s %s", args.http_method().c_str(), args.uri().c_str()));
            this->Log(L"");
            Log(StringFormat(L"[Response]: %d %s", args.http_status(), args.response_body().c_str()));
        }
    });

    UserInfoLabel->Text = L"Sign in succeeded";
    Log(this->UserInfoLabel->Text);

    // In order to prevent Windows Runtime STA threads from blocking the UI, calling task.wait() task.get() is illegal if task has not been completed.
    // and since OnSignInSucceeded is called from a task compleation using task_continuation_context::use_current(), it is running the STA thread so a new thread must be spun up here.
    pplx::create_task([this]()
    {
        m_scenarios.CreateAndJoinMultiplayerSession(this, m_xboxLiveContext);
    });
}

bool MainPage::CheckInternetAccess()
{
    bool hasInternetAccess = false;

    Windows::Networking::Connectivity::ConnectionProfile^ connectionProfile = Windows::Networking::Connectivity::NetworkInformation::GetInternetConnectionProfile();
    if (connectionProfile != nullptr)
    {
        auto connectivityLevel = connectionProfile->GetNetworkConnectivityLevel();
        if (connectivityLevel == Windows::Networking::Connectivity::NetworkConnectivityLevel::ConstrainedInternetAccess ||
            connectivityLevel == Windows::Networking::Connectivity::NetworkConnectivityLevel::InternetAccess)
        {
            hasInternetAccess = true;
        }
    }
    return hasInternetAccess;
}

void MainPage::UpdateInternetAccessUI()
{
    String^ newNetworkInfoTest = CheckInternetAccess() ? "Yes" : "No Internet";

    // Dispatch UI change to UI thread
    Dispatcher->RunAsync(
        Windows::UI::Core::CoreDispatcherPriority::Normal,
        ref new Windows::UI::Core::DispatchedHandler([this, newNetworkInfoTest]()
    {
        NetworkInfoLabel->Text = newNetworkInfoTest;

        Log(StringFormat(L"Network connectivity changed to %s ", newNetworkInfoTest->Data()));
    }));
}