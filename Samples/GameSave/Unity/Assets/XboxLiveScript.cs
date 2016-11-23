using UnityEngine;
using System;
#if NETFX_CORE
using Microsoft.Xbox.Services.System;
#endif

public class XboxLiveScript : MonoBehaviour
{
#if NETFX_CORE
    XboxLiveSignInUI xblSignInUI = new XboxLiveSignInUI();
    Microsoft.Xbox.Services.XboxLiveContext m_xboxLiveContext = null;
    XboxLiveGameSaveUI xblGameSaveUI = new XboxLiveGameSaveUI();
#endif

    void Start()
    {
#if NETFX_CORE
        xblSignInUI.OnUserSignedIn += XblSignInUI_OnUserSignedIn;
        xblSignInUI.OnUserSignedOut += XblSignInUI_OnUserSignedOut;
        xblSignInUI.Start();
#endif
    }

#if NETFX_CORE
    private void XblSignInUI_OnUserSignedIn(object sender, XboxLiveUser user)
    {
        m_xboxLiveContext = new Microsoft.Xbox.Services.XboxLiveContext(user);
        xblGameSaveUI.InitializeSaveSystem(m_xboxLiveContext);

        // In a real game, you should also call xblGameSaveUI.InitializeSaveSystem when resuming from
        // suspension, or you may encounter ObjectExpired errors.

#if DEBUG
        m_xboxLiveContext.Settings.EnableServiceCallRoutedEvents = true;
        m_xboxLiveContext.Settings.DiagnosticsTraceLevel = Microsoft.Xbox.Services.XboxServicesDiagnosticsTraceLevel.Verbose;
        m_xboxLiveContext.Settings.ServiceCallRouted += Settings_ServiceCallRouted;
#endif
    }

#if DEBUG
    private void Settings_ServiceCallRouted(object sender, Microsoft.Xbox.Services.XboxServiceCallRoutedEventArgs args)
    {
        System.Diagnostics.Debug.WriteLine(args.FullResponseToString);
    }
#endif

    private void XblSignInUI_OnUserSignedOut(object sender, XboxLiveUser user)
    {
        m_xboxLiveContext = null;
        xblGameSaveUI.InitializeSaveSystem(null);
    }
#endif
    
    void OnGUI()
    {
#if NETFX_CORE
        xblSignInUI.OnGUI();
        xblGameSaveUI.OnGUI();
#endif
    }
}
