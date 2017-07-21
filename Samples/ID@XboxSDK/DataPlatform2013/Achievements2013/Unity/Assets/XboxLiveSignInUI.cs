#if NETFX_CORE
using UnityEngine;
using System;
using System.Collections.Generic;
using Microsoft.Xbox.Services.System;

public class XboxLiveSignInUI
{
    Microsoft.Xbox.Services.System.XboxLiveUser m_user = new Microsoft.Xbox.Services.System.XboxLiveUser();
    Windows.UI.Core.CoreDispatcher m_uiDispatcher = null;
    GUIStyle m_guiStyle = new GUIStyle();
    string m_logText = string.Empty;
    List<string> m_logLines = new List<string>();

    public void Start()
    {
        Windows.ApplicationModel.Core.CoreApplicationView mainView = Windows.ApplicationModel.Core.CoreApplication.MainView;
        Windows.UI.Core.CoreWindow cw = mainView.CoreWindow;
        XboxLiveUser.SignOutCompleted += OnUserSignOut;

        m_uiDispatcher = cw.Dispatcher;
        SignInSilent();
    }

    public void OnUserSignOut(object sender, SignOutCompletedEventArgs e)
    {
        LogLine("User signed out: " + e.User.Gamertag);
        OnUserSignedOut(this, e.User);
    }

    public bool IsSignedIn()
    {
        return m_user.IsSignedIn;
    }

    public Microsoft.Xbox.Services.System.XboxLiveUser User
    {
        get { return m_user; }
    }

    public event EventHandler<Microsoft.Xbox.Services.System.XboxLiveUser> OnUserSignedOut;
    public event EventHandler<Microsoft.Xbox.Services.System.XboxLiveUser> OnUserSignedIn;

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
            DrawTextWithShadow(10, 50, 800, 500, m_logText);
        }

        if (GUI.Button(new Rect(10, 10, 100, 30), "Sign-in"))
        {
            SignIn();
        }
    }

    public void LogLine(string line)
    {
        lock (m_logText)
        {
            if (m_logLines.Count > 5)
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
        }
    }

    async public void SignInSilent()
    {
        try
        {
            SignInResult result = await m_user.SignInSilentlyAsync(m_uiDispatcher);
            HandleSignInResult(result);
        }
        catch (System.Exception ex)
        {
            LogLine("SignInSilent failed: " + ex.ToString());
        }
    }

    async public void SignIn()
    {
        try
        {
            SignInResult result = await m_user.SignInAsync(m_uiDispatcher);
            HandleSignInResult(result);
        }
        catch (System.Exception ex)
        {
            LogLine("SignIn failed: " + ex.ToString());
        }
    }

    private void HandleSignInResult(SignInResult result)
    {
        switch (result.Status)
        {
            case SignInStatus.Success:
                LogLine("User signed in: " + m_user.Gamertag);
                OnUserSignedIn(this, m_user);
                break;

            case SignInStatus.UserCancel:
                LogLine("User cancelled during sign in");
                break;

            default:
                LogLine("Unknown error during sign in");
                break;
        }
    }
}

#endif
