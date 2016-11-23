using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Microsoft.Xbox.Services;
using Microsoft.Xbox.Services.System;
using Microsoft.Xbox.Services.Social;
using Microsoft.Xbox.Services.Leaderboard;

namespace Social
{
    class Scenarios
    {
        async public void Scenario_GetUserProfileAsync(MainPage ui, Microsoft.Xbox.Services.XboxLiveContext xboxLiveContext)
        {
            ui.Log("Calling GetUserProfileAsync...");

            try
            {
                Microsoft.Xbox.Services.Social.XboxUserProfile profile = await xboxLiveContext.ProfileService.GetUserProfileAsync(xboxLiveContext.User.XboxUserId);

                lock (ui)
                {
                    ui.Log("");
                    ui.Log("----------------");
                    ui.Log("GetProfileAsync results:");
                    ui.Log("");
                    ui.Log("Gamerscore: " + profile.Gamerscore);
                    ui.Log("Gamertag: " + profile.Gamertag);
                    ui.Log("XboxUserId: " + profile.XboxUserId);
                    ui.Log("ApplicationDisplayName: " + profile.ApplicationDisplayName);
                    ui.Log("ApplicationDisplayPictureResizeUri: " + profile.ApplicationDisplayPictureResizeUri.AbsoluteUri);
                    //ui.Log("DisplayPictureUri: " + profile.DisplayPictureUri.AbsoluteUri);
                    ui.Log("GameDisplayName: " + profile.GameDisplayName);
                    ui.Log("GameDisplayPictureResizeUri: " + profile.GameDisplayPictureResizeUri.AbsoluteUri);
                    //ui.Log("PublicGamerPictureUri: " + profile.PublicGamerPictureUri.AbsoluteUri);
                }
            }
            catch(Exception e)
            {
                ui.Log("Call failed. Exception details: " + e.ToString());
            }
        }
        
        async public void Scenario_GetUserProfilesForSocialGroupAsync(
            MainPage ui, 
            Microsoft.Xbox.Services.XboxLiveContext xboxLiveContext
            )
        {
            ui.Log("Calling GetUserProfilesForSocialGroupAsync...");

            try
            {
                var result = await xboxLiveContext.ProfileService.GetUserProfilesForSocialGroupAsync("People");

                lock (ui)
                {
                    ui.Log("");
                    ui.Log("----------------");
                    ui.Log("GetUserProfilesForSocialGroupAsync results:");
                    ui.Log("");
                    int index = 1;
                    foreach (Microsoft.Xbox.Services.Social.XboxUserProfile profile in result)
                    {
                        ui.Log("");
                        ui.Log(string.Format("[Item {0}]", index));
                        ui.Log("Gamerscore: " + profile.Gamerscore);
                        ui.Log("Gamertag: " + profile.Gamertag);
                        ui.Log("XboxUserId: " + profile.XboxUserId);
                        index++;
                    }
                }
            }
            catch (Exception e)
            {
                ui.Log("GetUserProfilesForSocialGroupAsync failed. Exception details: " + e.ToString());
            }
        }

        async public void Scenario_PeoplePickerAsyncTask(
            MainPage ui, 
            Microsoft.Xbox.Services.XboxLiveContext xboxLiveContext
            )
        {
            ui.Log("Calling ShowPlayerPickerUI...");

            try
            {
                var result = await Microsoft.Xbox.Services.System.TitleCallableUI.ShowPlayerPickerUI(
                    "Choose people now!", // prompt text
                    new List<string>() { "2814659809402826", "2814613569642996", "2814632956486799" }, // list of XboxUserIds to show
                    new List<string>() { "2814613569642996" }, // list of preselected XboxUserIds 
                    1, // min count of selection
                    2  // max count of selection
                    );

                foreach (string selectedXuid in result)
                {
                    ui.Log("Selected: " + selectedXuid);
                }
            }
            catch (Exception ex)
            {
                ui.Log("ShowPlayerPickerUI failed. Exception details: " + ex.ToString());
            }
        }

        Microsoft.Xbox.Services.Multiplayer.MultiplayerSession m_session = null;        
        async public void Scenario_SendInviteAsyncTask(
            MainPage ui,
            Microsoft.Xbox.Services.XboxLiveContext xboxLiveContext
            )
        {
            try
            {
                if (m_session == null)
                {
                    m_session = await CreateAndJoinMultiplayerSession(xboxLiveContext);
                    await xboxLiveContext.MultiplayerService.SetActivityAsync(m_session.SessionReference);
                }

                ui.Log("Calling ShowGameInviteUIAsync...");

                try
                {
                    await Microsoft.Xbox.Services.System.TitleCallableUI.ShowGameInviteUIAsync(
                        m_session.SessionReference,
                        "Join my game!"
                        );

                    ui.Log("Invite sent");
                }
                catch (Exception ex)
                {
                    ui.Log("ShowGameInviteUIAsync failed. Exception details: " + ex.ToString());
                }
            }
            catch (Exception ex)
            {
                ui.Log("CreateAndJoinMultiplayerSession failed. Exception details: " + ex.ToString());
            }
        }

        async private Task<Microsoft.Xbox.Services.Multiplayer.MultiplayerSession> CreateAndJoinMultiplayerSession(
            Microsoft.Xbox.Services.XboxLiveContext xboxLiveContext
            )
        {
            string sessionId = "testSession1";
            string sessionTemplateName = "GameSession10PublicNoActive";
            Microsoft.Xbox.Services.Multiplayer.MultiplayerSessionReference sessionRef = new Microsoft.Xbox.Services.Multiplayer.MultiplayerSessionReference(
                xboxLiveContext.AppConfig.ServiceConfigurationId,
                sessionTemplateName,
                sessionId
                );

            Microsoft.Xbox.Services.Multiplayer.MultiplayerSession session = new Microsoft.Xbox.Services.Multiplayer.MultiplayerSession(
                xboxLiveContext,
                sessionRef
                );
            session.Join("", true, true);

            return await xboxLiveContext.MultiplayerService.WriteSessionAsync(session, Microsoft.Xbox.Services.Multiplayer.MultiplayerSessionWriteMode.UpdateOrCreateNew);
        }

        async public void Scenario_ShowProfileCardUI(
            MainPage ui, 
            Microsoft.Xbox.Services.XboxLiveContext xboxLiveContext
            )
        {
            ui.Log("Calling ShowProfileCardUIAsync...");

            try
            {
                await Microsoft.Xbox.Services.System.TitleCallableUI.ShowProfileCardUIAsync(
                    "2814613569642996" 
                    );

                ui.Log("ShowProfileCardUIAsync done.");
            }
            catch (Exception ex)
            {
                ui.Log("ShowProfileCardUIAsync failed. Exception details: " + ex.ToString());
            }
        }

        async public void Scenario_ShowChangeFriendRelationshipUI(
            MainPage ui, 
            Microsoft.Xbox.Services.XboxLiveContext xboxLiveContext
            )
        {
            ui.Log("Calling ShowChangeFriendRelationshipUIAsync...");

            try
            {
                await Microsoft.Xbox.Services.System.TitleCallableUI.ShowChangeFriendRelationshipUIAsync(
                    "2814613569642996" 
                    );

                ui.Log("ShowChangeFriendRelationshipUIAsync done.");
            }
            catch (Exception ex)
            {
                ui.Log("ShowChangeFriendRelationshipUIAsync failed. Exception details: " + ex.ToString());
            }
        }

        async public void Scenario_ShowTitleAchievementsUI(
            MainPage ui,
            Microsoft.Xbox.Services.XboxLiveContext xboxLiveContext
            )
        {
            ui.Log("Calling ShowTitleAchievementsUIAsync...");

            try
            {
                await Microsoft.Xbox.Services.System.TitleCallableUI.ShowTitleAchievementsUIAsync(
                    xboxLiveContext.AppConfig.TitleId
                    );

                ui.Log("ShowTitleAchievementsUIAsync done.");
            }
            catch (Exception ex)
            {
                ui.Log("ShowTitleAchievementsUIAsync failed. Exception details: " + ex.ToString());
            }
        }
    }
}
