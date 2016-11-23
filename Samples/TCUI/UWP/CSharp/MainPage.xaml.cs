//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

using System.Threading;
using System.Threading.Tasks;
using Microsoft.Xbox.Services.System;

namespace Social
{
    public enum ScenarioItemTag
    {
        Scenario_GetUserProfileAsync = 1,
        Scenario_GetUserProfilesForSocialGroupAsync,

        Scenario_PeoplePickerAsyncTask,
        Scenario_SendInviteAsyncTask,
        Scenario_ShowProfileCardUI,
        Scenario_ShowChangeFriendRelationshipUI,
        Scenario_ShowTitleAchievementsUI
    };

    struct ScenarioDescriptionItem
    {
        public ScenarioItemTag tag;
        public string name;
    };

    public sealed partial class MainPage : Page
    {
        Microsoft.Xbox.Services.System.XboxLiveUser m_user = new Microsoft.Xbox.Services.System.XboxLiveUser();
        Microsoft.Xbox.Services.XboxLiveContext m_xboxLiveContext = null;
        Scenarios m_scenarios = new Scenarios();
        Windows.UI.Core.CoreDispatcher UIDispatcher = null;


        static ScenarioDescriptionItem[] ScenarioDescriptions =
            new ScenarioDescriptionItem[]
            {
                new ScenarioDescriptionItem() { tag = ScenarioItemTag.Scenario_GetUserProfileAsync, name = "Get user profile" },
                new ScenarioDescriptionItem() { tag = ScenarioItemTag.Scenario_GetUserProfilesForSocialGroupAsync, name = "Get social list for social group" },
                new ScenarioDescriptionItem() { tag = ScenarioItemTag.Scenario_PeoplePickerAsyncTask, name = "Show People Picker"},
                new ScenarioDescriptionItem() { tag = ScenarioItemTag.Scenario_SendInviteAsyncTask, name = "Send Invite"},
                new ScenarioDescriptionItem() { tag = ScenarioItemTag.Scenario_ShowProfileCardUI, name = "Show Profile Card UI" },
                new ScenarioDescriptionItem() { tag = ScenarioItemTag.Scenario_ShowChangeFriendRelationshipUI, name = "Show Change Friend Relationship UI" },
                new ScenarioDescriptionItem() { tag = ScenarioItemTag.Scenario_ShowTitleAchievementsUI, name = "Show Title Achievements UI" }
            };

        private bool RunScenario(ScenarioItemTag selectedTag)
        {
            if (m_xboxLiveContext == null || !m_xboxLiveContext.User.IsSignedIn)
            {
                Log("No user signed in");
                return false;
            }

            switch (selectedTag)
            {
                case ScenarioItemTag.Scenario_GetUserProfileAsync: 
                    m_scenarios.Scenario_GetUserProfileAsync(this, m_xboxLiveContext); 
                    break;
                case ScenarioItemTag.Scenario_GetUserProfilesForSocialGroupAsync: 
                    m_scenarios.Scenario_GetUserProfilesForSocialGroupAsync(this, m_xboxLiveContext); 
                    break;

                case ScenarioItemTag.Scenario_PeoplePickerAsyncTask:
                    m_scenarios.Scenario_PeoplePickerAsyncTask(this, m_xboxLiveContext);
                    break;

                case ScenarioItemTag.Scenario_SendInviteAsyncTask:
                    m_scenarios.Scenario_SendInviteAsyncTask(this, m_xboxLiveContext);
                    break;

                case ScenarioItemTag.Scenario_ShowProfileCardUI:
                    m_scenarios.Scenario_ShowProfileCardUI(this, m_xboxLiveContext);
                    break;

                case ScenarioItemTag.Scenario_ShowChangeFriendRelationshipUI:
                    m_scenarios.Scenario_ShowChangeFriendRelationshipUI(this, m_xboxLiveContext);
                    break;

                case ScenarioItemTag.Scenario_ShowTitleAchievementsUI:
                    m_scenarios.Scenario_ShowTitleAchievementsUI(this, m_xboxLiveContext);
                    break;

                default:
                    return false;
            }

            return true;
        }

        public MainPage()
        {
            this.InitializeComponent();
            UIDispatcher = Windows.UI.Xaml.Window.Current.CoreWindow.Dispatcher;
            XboxLiveUser.SignOutCompleted += XboxLiveUser_SignOutCompleted;

            foreach (ScenarioDescriptionItem scenario in ScenarioDescriptions)
            {
                ListBoxItem listBoxItem = new ListBoxItem();
                listBoxItem.Content = scenario.name;
                listBoxItem.Tag = (object)scenario.tag;
                ScenarioListBox.Items.Add(listBoxItem);
            }

            ScenarioListBox.SelectedIndex = 0;

            XboxLiveUser.SignOutCompleted += XboxLiveUser_SignOutCompleted;
            Windows.Networking.Connectivity.NetworkInformation.NetworkStatusChanged += new Windows.Networking.Connectivity.NetworkStatusChangedEventHandler(
                (object sender) =>
                {
                    UpdateInternetAccessUI();
                });

            UpdateInternetAccessUI();
        }

        private async void XboxLiveUser_SignOutCompleted(object sender, SignOutCompletedEventArgs args)
        {
            await UIDispatcher.RunAsync(
                Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
                {
                    UserInfoLabel.Text = "User signed out";
                    Log("User " + args.User.Gamertag + " signed out.");
                });
        }

        private void ScenarioListBox_DoubleTapped(object sender, DoubleTappedRoutedEventArgs e)
        {
            RunSelectedScenario();
        }

        private void RunButton_Click(object sender, RoutedEventArgs e)
        {
            RunSelectedScenario();
        }

        private void SignInButton_Click(object sender, RoutedEventArgs e)
        {
            SignIn();
        }
        private void SignInSilentButton_Click(object sender, RoutedEventArgs e)
        {
            SignInSilently();
        }

        private void SwitchAccountButton_Click(object sender, RoutedEventArgs e)
        {
            SwitchAccount();
        }

        void ClearLogs()
        {
            OutputStackPanel.Children.Clear();
        }

        public void Log(string logLine)
        {
            TextBlock uiElement = new TextBlock();
            uiElement.FontSize = 14;
            uiElement.Text = logLine;
            OutputStackPanel.Children.Add(uiElement);
            System.Diagnostics.Debug.WriteLine(logLine);
        }

        private void RunAllButton_Click(object sender, RoutedEventArgs e)
        {
            ClearLogs();
            int scenarioTag = 1;
            for (; ; )
            {
                bool success = RunScenario((ScenarioItemTag)scenarioTag);
                if (!success)
                {
                    break;
                }

                scenarioTag++;
            }
        }

        private void RunSelectedScenario()
        {
            if (ScenarioListBox.SelectedItems.Count == 1)
            {
                Log("");
                Log("--------------");
                ListBoxItem selectedItem = (ListBoxItem)(ScenarioListBox.SelectedItem);
                ScenarioItemTag selectedTag = (ScenarioItemTag)selectedItem.Tag;
                RunScenario(selectedTag);
            }
        }

        async void SignIn()
        {
            UserInfoLabel.Text = "Trying to sign in...";
            Log(UserInfoLabel.Text);

            try
            {
                var signInResult = await m_user.SignInAsync(UIDispatcher);
                switch (signInResult.Status)
                {
                    case SignInStatus.Success:
                        m_xboxLiveContext = new Microsoft.Xbox.Services.XboxLiveContext(m_user);
                        UserInfoLabel.Text = "Sign in succeeded";
                        break;
                    case SignInStatus.UserCancel:
                        UserInfoLabel.Text = "User cancel";
                        break;
                    default:
                        UserInfoLabel.Text = "Unknown Error";
                        break;
                }
                Log(UserInfoLabel.Text);
            }
            catch (Exception e)
            {
                Log("Sign in failed");
                Log("SignInAsync failed.  Exception: " + e.ToString());
            }
        }

        async void SignInSilently()
        {
            UserInfoLabel.Text = "Trying to sign in silently...";
            Log(UserInfoLabel.Text);

            try
            {
                var signInResult = await m_user.SignInSilentlyAsync(UIDispatcher);
                switch (signInResult.Status)
                {
                    case SignInStatus.Success:
                        m_xboxLiveContext = new Microsoft.Xbox.Services.XboxLiveContext(m_user);
                        UserInfoLabel.Text = "Sign in succeeded";
                        break;
                    case SignInStatus.UserInteractionRequired:
                        UserInfoLabel.Text = "User interaction required";
                        break;
                    default:
                        UserInfoLabel.Text = "Unknown Error";
                        break;
                }
                Log(UserInfoLabel.Text);
            }
            catch (Exception e)
            {
                Log("Sign in failed");
                Log("SignInSilentlyAsync failed.  Exception: " + e.ToString());
            }
        }

        async void SwitchAccount()
        {
            UserInfoLabel.Text = "Trying to Switch Account...";
            Log(UserInfoLabel.Text);

            try
            {
                var signInResult = await m_user.SwitchAccountAsync(Windows.UI.Xaml.Window.Current.CoreWindow.Dispatcher);
                switch (signInResult.Status)
                {
                    case SignInStatus.Success:
                        m_xboxLiveContext = new Microsoft.Xbox.Services.XboxLiveContext(m_user);
                        UserInfoLabel.Text = "Switch account succeeded";
                        break;
                    case SignInStatus.UserCancel:
                        UserInfoLabel.Text = "User cancel";
                        break;
                    default:
                        UserInfoLabel.Text = "Unknown Error";
                        break;
                }
                Log(UserInfoLabel.Text);
            }
            catch (Exception e)
            {
                Log("Switch account failed");
                Log("SwitchAccountAsync failed.  Exception: " + e.ToString());
            }
        }
        private bool CheckInternetAccess()
        {
            bool internetAccess = false;
            Windows.Networking.Connectivity.ConnectionProfile connectionProfile = Windows.Networking.Connectivity.NetworkInformation.GetInternetConnectionProfile();

            if (connectionProfile != null)
            {
                var connectivityLevel = connectionProfile.GetNetworkConnectivityLevel();
                if (connectivityLevel == Windows.Networking.Connectivity.NetworkConnectivityLevel.ConstrainedInternetAccess ||
                    connectivityLevel == Windows.Networking.Connectivity.NetworkConnectivityLevel.InternetAccess)
                {
                    internetAccess = true;
                }
            }

            return internetAccess;
        }

        private async void UpdateInternetAccessUI()
        {
            String newNetworkInfoTest = CheckInternetAccess() ? "Yes" : "No Internet";

            // Dispatch UI change to UI thread
            await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                NetworkInfoLabel.Text = newNetworkInfoTest;

                Log("Network connectivity changed to " + newNetworkInfoTest);
            });
        }
    }
}
