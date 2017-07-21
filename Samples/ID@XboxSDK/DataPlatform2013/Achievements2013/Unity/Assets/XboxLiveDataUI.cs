#if NETFX_CORE
using UnityEngine;
using System;
using System.Collections.Generic;
using Windows.Foundation.Collections;
using Microsoft.Xbox.Services;
using Microsoft.Xbox.Services.Achievements;
using Microsoft.Xbox.Services.Leaderboard;

public class XboxLiveDataUI
{
    XboxLiveContext m_context;
    GUIStyle m_guiStyle = new GUIStyle();
    string m_logText = string.Empty;
    List<string> m_logLines = new List<string>();
    
    public void SetXboxLiveContext(XboxLiveContext context)
    {
        m_context = context;
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
            DrawTextWithShadow(10, 270, 800, 900, m_logText);
        }

        if (GUI.Button(new Rect(10, 150, 150, 30), "Send Event"))
        {
            SendEvent();
        }

        if (GUI.Button(new Rect(10, 190, 150, 30), "Get Achievement"))
        {
            GetAchievement();
        }

        if (GUI.Button(new Rect(10, 230, 150, 30), "Get Leaderboard"))
        {
            GetLeaderboard();
        }
    }

    private void SendEvent()
    {
        if (m_context == null)
        {
            LogLine("A user is required to send an event. Please sign in a user first.");
            LogLine("");
        }
        else
        {
            try
            {
                PropertySet properties = new PropertySet();
                properties["MultiplayerCorrelationId"] = "multiplayer correlation id";
                properties["GameplayModeId"] = 1;
                properties["DifficultyLevelId"] = 100;
                properties["RoundId"] = 1;
                properties["PlayerRoleId"] = 1;
                properties["PlayerWeaponId"] = 2;
                properties["EnemyRoleId"] = 3;
                properties["KillTypeId"] = 4;

                PropertySet measurements = new PropertySet();
                measurements["LocationX"] = 1;
                measurements["LocationY"] = 2.12121;
                measurements["LocationZ"] = -90909093;

                m_context.EventsService.WriteInGameEvent("EnemyDefeated", properties, measurements);
                LogLine("EnemyDefeated event was fired.");
                LogLine("");
            }
            catch(Exception ex)
            {
                LogLine("SendEvent failed: " + ex.Message);
                LogLine("");
            }
        }
    }

    private async void GetAchievement()
    {
        if (m_context == null)
        {
            LogLine("A user is required to get achievements. Please sign in a user first.");
            LogLine("");
        }
        else
        {
            LogLine("Getting achievement...");
            LogLine("");

            try
            {
                Achievement achievement = await m_context.AchievementService.GetAchievementAsync(m_context.User.XboxUserId,
                    m_context.AppConfig.ServiceConfigurationId, "1");

                bool unlocked = (achievement.ProgressState == AchievementProgressState.Achieved);
                LogLine(String.Format("Achievement: {0} (ID: {1})", achievement.Name, achievement.Id));
                LogLine(String.Format("Description: {0}", unlocked ? achievement.UnlockedDescription : achievement.LockedDescription));
                LogLine(String.Format("AchievementType: {0}", achievement.AchievementType));
                LogLine(String.Format("ProgressState: {0}", achievement.ProgressState));
                LogLine("");
            }
            catch (Exception ex)
            {
                LogLine("GetAchievement failed: " + ex.Message);
                LogLine("");
            }
        }
    }

    private async void GetLeaderboard()
    {
        if (m_context == null)
        {
            LogLine("A user is required to get leaderboards. Please sign in a user first.");
            LogLine("");
        }
        else
        {
            LogLine("Getting leaderboard...");
            LogLine("");

            try
            {
                LeaderboardResult result = await m_context.LeaderboardService.GetLeaderboardAsync(
                    m_context.AppConfig.ServiceConfigurationId, "LBEnemyDefeatsDescending");

                LogLeaderboardResult(result);

                // A single call to GetLeaderboardAsync only returns the top ten entries, so let's
                // request a couple more pages.
                const int maxResultsToLog = 3;
                int resultsLogged = 1;

                while(result.HasNext && resultsLogged < maxResultsToLog)
                {
                    result = await result.GetNextAsync(10);
                    LogLeaderboardResult(result);

                    resultsLogged++;
                }                
            }
            catch (Exception ex)
            {
                LogLine("GetLeaderboard failed: " + ex.Message);
                LogLine("");
            }
        }
    }

    private void LogLeaderboardResult(LeaderboardResult result)
    {
        LogLine("Columns: " + GetColumnsString(result));

        foreach (LeaderboardRow row in result.Rows)
        {
            LogLine(String.Format("Gamertag: {0}, Rank: {1}, Percentile: {2}, Values: {3}",
                row.Gamertag, row.Rank, row.Percentile, GetRowValuesString(row)));
        }

        LogLine("");
    }

    private string GetColumnsString(LeaderboardResult result)
    {
        string output = "";

        foreach (LeaderboardColumn column in result.Columns)
        {
            output += (column.DisplayName + ", ");
        }

        return output;
    }

    private string GetRowValuesString(LeaderboardRow row)
    {
        string output = "";

        foreach (string val in row.Values)
        {
            output += (val + ", ");
        }

        return output;
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
            if (m_logLines.Count > 40)
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
