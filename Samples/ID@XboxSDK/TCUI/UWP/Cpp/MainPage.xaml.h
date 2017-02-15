//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
#pragma once

#include "MainPage.g.h"
#include "Scenarios.h"

Platform::String^ StringFormat(LPCWSTR strMsg, ...);

namespace Sample
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public ref class MainPage sealed
    {
    public:
        MainPage();

    internal:
        void ClearLogs();
        void Log(Platform::String^ logLine);
        property Windows::UI::Core::CoreDispatcher^ CoreDispatcher;

    private:
        void ScenarioListBox_DoubleTapped(Platform::Object^ sender, Windows::UI::Xaml::Input::DoubleTappedRoutedEventArgs^ e);
        void RunButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void RunAllButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void SignInButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        void SwitchAccountButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

        void SignIn();
        void SignInSilently();
        void SwitchAccount();
        void OnSignInSucceeded();

        void RunSelectedScenario();
        bool RunScenario(int selectedTag);

        bool CheckInternetAccess();
        void UpdateInternetAccessUI();

        std::shared_ptr<xbox::services::system::xbox_live_user> m_user;
        std::shared_ptr<xbox::services::xbox_live_context> m_xboxLiveContext;
        Scenarios m_scenarios;
    };
}
