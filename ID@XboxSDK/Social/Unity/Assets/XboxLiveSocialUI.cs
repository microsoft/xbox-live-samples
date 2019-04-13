#if NETFX_CORE
using UnityEngine;
using System;
using System.Collections.Generic;
using Microsoft.Xbox.Services;
using Microsoft.Xbox.Services.System;
using Microsoft.Xbox.Services.Social.Manager;
using Microsoft.Xbox.Services.Presence;

public class XboxLiveSocialUI
{
    SocialManager m_socialManager = SocialManager.SingletonInstance;
    XboxLiveUser m_user = null;
    XboxLiveContext m_context = null;
    XboxSocialUserGroup m_socialManagerUserGroup = null;

    GUIStyle m_guiStyle = new GUIStyle();
    string m_logText = string.Empty;
    List<string> m_logLines = new List<string>();

    // These are defined in XDP and map to rich presence strings.
    string[] m_presenceIds = { "rpdemo", "rpgame" };
    int m_currentPresenceId = 0;

    public void AddUser(XboxLiveUser user)
    {
        lock (m_socialManager)
        {
            if (m_user == null)
            {
                m_user = user;
                m_context = new XboxLiveContext(user);
                m_socialManager.AddLocalUser(user, SocialManagerExtraDetailLevel.NoExtraDetail);
                m_socialManagerUserGroup = m_socialManager.CreateSocialUserGroupFromFilters(m_user, PresenceFilter.All, RelationshipFilter.Friends);
                LogLine("Adding user from graph");
            }
        }
    }

    public void RemoveUser(XboxLiveUser user)
    {
        lock (m_socialManager)
        {
            if (m_socialManagerUserGroup != null)
            {
                m_socialManager.DestroySocialUserGroup(m_socialManagerUserGroup);
                m_socialManagerUserGroup = null;
            }

            if (m_user != null)
            {
                m_socialManager.RemoveLocalUser(m_user);
                m_user = null;
                LogLine("Removing user from graph");
            }

            m_context = null;
        }
    }

    public void Update()
    {
        m_socialManager.DoWork();
    }

    private void DrawTextWithShadow(float x, float y, float width, float height, string text)
    {
        m_guiStyle.fontSize = 14;
        m_guiStyle.normal.textColor = Color.black;
        GUI.Label(new UnityEngine.Rect(x, y, height, height), text, m_guiStyle);
        m_guiStyle.normal.textColor = Color.white;
        GUI.Label(new UnityEngine.Rect(x - 1, y - 1, width, height), text, m_guiStyle);
    }

    public void OnGUI()
    {
        lock (m_logText)
        {
            DrawTextWithShadow(10, 300, 800, 900, m_logText);
        }

        ClearLog();

        if (GUI.Button(new Rect(10, 150, 150, 30), "Cycle Presence"))
        {
            CyclePresence();
        }

        lock (m_socialManager)
        {
            if (m_socialManagerUserGroup != null)
            {
                IReadOnlyList<XboxSocialUser> userList = m_socialManagerUserGroup.Users;
                LogLine("Friends:");

                if (userList.Count == 0)
                {
                    LogLine("No friends found");
                }
                else
                {
                    foreach (XboxSocialUser socialUser in userList)
                    {
                        LogFriend(socialUser);
                    }
                }

            }
        }
    }

    private void LogFriend(XboxSocialUser socialUser)
    {
        LogLine("Gamertag: " + socialUser.Gamertag + ". Status: " + socialUser.PresenceRecord.UserState.ToString());

        // Get information on what the user is playing. A user may have presence on multiple devices and titles.

        foreach (SocialManagerPresenceTitleRecord titleRecord in socialUser.PresenceRecord.PresenceTitleRecords)
        {
            LogLine("  Title: " + titleRecord.TitleId + "  Presence: " + titleRecord.PresenceText);
        }
    }

    private async void CyclePresence()
    {
        m_currentPresenceId = (m_currentPresenceId + 1) % m_presenceIds.Length;

        try
        {
            PresenceData data = new PresenceData(m_context.AppConfig.ServiceConfigurationId, m_presenceIds[m_currentPresenceId]);
            await m_context.PresenceService.SetPresenceAsync(true, data);
        }
        catch(Exception ex)
        {
            LogLine("CyclePresence failed: " + ex.ToString());
        }
    }

    public void ClearLog()
    {
        lock (m_logText)
        {
            m_logLines.Clear();
        }
    }

    public void LogLine(string line)
    {
        lock (m_logText)
        {
            if (m_logLines.Count > 20)
            {
                m_logLines.RemoveAt(0);
            }
            m_logLines.Add(line);

            m_logText = string.Empty;
            foreach (string s in m_logLines)
            {
                m_logText += "\n";
                m_logText += s;
            }
            m_logText += "\n";
        }
    }
}

#endif
