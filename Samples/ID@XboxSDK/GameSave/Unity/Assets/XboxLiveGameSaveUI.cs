#if NETFX_CORE
using UnityEngine;
using System;
using System.Collections.Generic;
using Microsoft.Xbox.Services;
using Windows.Gaming.XboxLive.Storage;

public class XboxLiveGameSaveUI
{
    XboxLiveContext m_context;
    GUIStyle m_guiStyle = new GUIStyle();
    string m_logText = string.Empty;
    List<string> m_logLines = new List<string>();
    System.Random m_random = new System.Random();
    int m_gameData = 0;
    GameSaveManager m_saveManager = new GameSaveManager();
    
    public async void InitializeSaveSystem(XboxLiveContext context)
    {
        m_context = context;

        if (context == null)
        {
            LogLine("Resetting save system.");
            LogLine("");

            m_saveManager.Reset();
        }
        else
        {
            try
            {
                LogLine("Initializing save system...");
                GameSaveErrorStatus status = await m_saveManager.Initialize(context);

                if (status == GameSaveErrorStatus.Ok)
                {
                    LogLine("Successfully initialized save system.");
                }
                else
                {
                    LogLine(String.Format("InitializeSaveSystem failed: {0}", status));
                }
            }
            catch(Exception ex)
            {
                LogLine("InitializeSaveSystem failed: " + ex.Message);
            }

            LogLine("");
        }
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
            DrawTextWithShadow(10, 350, 800, 900, m_logText);
        }

        if (GUI.Button(new Rect(10, 150, 150, 30), "Generate Data"))
        {
            m_gameData = m_random.Next();

            LogLine(String.Format("Game data: {0}", m_gameData));
            LogLine("");
        }

        if (GUI.Button(new Rect(10, 190, 150, 30), "Save Data"))
        {
            SaveData();
        }

        if (GUI.Button(new Rect(10, 230, 150, 30), "Load Data"))
        {
            LoadData();
        }

        if (GUI.Button(new Rect(10, 270, 150, 30), "Get Container Info"))
        {
            GetContainerInfo();
        }

        if (GUI.Button(new Rect(10, 310, 150, 30), "Delete Container"))
        {
            DeleteContainer();
        }
    }

    private async void SaveData()
    {
        try
        {
            GameSaveErrorStatus status = await m_saveManager.SaveData(m_gameData);

            if (status == GameSaveErrorStatus.Ok)
            {
                LogLine(String.Format("Saved data : {0}", m_gameData));
            }
            else
            {
                LogLine(String.Format("SaveData failed: {0}", status));
            }
        }
        catch (Exception ex)
        {
            LogLine("SaveData failed: " + ex.Message);
        }

        LogLine("");
    }

    private async void LoadData()
    {
        try
        {
            LoadDataResult loadResult = await m_saveManager.LoadData();

            if (loadResult.Status == GameSaveErrorStatus.Ok)
            {
                m_gameData = loadResult.Data;
                LogLine(String.Format("Loaded data : {0}", m_gameData));
            }
            else
            {
                LogLine(String.Format("LoadData failed: {0}", loadResult.Status));
            }
        }
        catch (Exception ex)
        {
            LogLine("LoadData failed: " + ex.Message);
        }

        LogLine("");
    }

    private async void GetContainerInfo()
    {
        try
        {
            GameSaveContainerInfoGetResult result = await m_saveManager.GetContainerInfo();

            if (result.Status == GameSaveErrorStatus.Ok)
            {
                LogLine("Got container info:");
                LogLine("");
                LogSaveContainerInfoList(result.Value);
            }
            else
            {
                LogLine(String.Format("GetContainerInfo failed: {0}", result.Status));
            }
        }
        catch (Exception ex)
        {
            LogLine("GetContainerInfo failed: " + ex.Message);
        }

        LogLine("");
    }

    private void LogSaveContainerInfoList(IReadOnlyList<GameSaveContainerInfo> list)
    {
        if(list.Count == 0)
        {
            LogLine("[Empty GameSaveContainerInfo list]");
            LogLine("");
        }

        for(int i = 0; i < list.Count; i++)
        {
            LogLine(String.Format("Container #{0}", i));
            LogLine("Name: " + list[i].Name);
            LogLine("DisplayName: " + list[i].DisplayName);
            LogLine(String.Format("LastModifiedTime: {0}", list[i].LastModifiedTime));
            LogLine(String.Format("TotalSize: {0}", list[i].TotalSize));
            LogLine(String.Format("NeedsSync: {0}", list[i].NeedsSync));
            LogLine("");
        }
    }

    private async void DeleteContainer()
    {
        try
        {
            GameSaveErrorStatus status = await m_saveManager.DeleteContainer();

            if (status == GameSaveErrorStatus.Ok)
            {
                LogLine("Deleted save container.");
            }
            else
            {
                LogLine(String.Format("DeleteContainer failed: {0}", status));
            }
        }
        catch (Exception ex)
        {
            LogLine("DeleteContainer failed: " + ex.Message);
        }

        LogLine("");
    }

    public void LogLine(string line)
    {
        lock (m_logText)
        {
            if (m_logLines.Count > 30)
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
