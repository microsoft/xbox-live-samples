How to run the Unity sample
---------------------------

1. Open the sample in Unity. Unity must be installed with the "Windows Store .NET Scripting backend" or "Windows Store IL2CPP Scripting backend" component
1. Open the scene Assets\Game.unity
1. Drag & drop "XboxLiveScript" from Assets window onto the Main Camera in the Hierarchy window
1. Drag & drop the Microsoft.Xbox.Services.winmd from https://aka.ms/xboxlivesdkunity into the assets tab
1. Build the project in Unity.
	1. Go to File | Build Settings, click Windows Store and click "Switch Platform"
	1. Click "Add Open Scenes" to add the current scene called "Game" to the build
	1. In the SDK combo box, choose "Universal 10" 
	1. In the UWP build type combo box, choose "D3D"
	1. Click the "Unity C\# Projects" checkbox to generate the Assembly-Csharp.dll projects
	1. Click "Build" for Unity to generate the UWP Visual Studio project that wraps your Unity game in a UWP application. 
	   When you get prompted for a location, select the existing "Build" folder.  This folder contains a Package.appxmanifest and xboxservices.config that are used below.
1. Open Unity.sln from the \Build folder of the sample
1. Add the "Microsoft.Xbox.Live.SDK.WinRT.UWP" NuGet package references
	1. Follow https://developer.microsoft.com/en-us/games/xbox/docs/xboxlive/get-started/uwp/adding-xbox-live-apis-binary-package-to-your-uwp-project to add the nuget package references.   
	1. Add NuGet package references to the the UWP app project called "Unity (Universal Windows)"
1. Add xboxservices.config to the "Unity (Universal Windows)" project
	1. Add the existing xboxservices.config from the root of the sample
	1. In the Properties window for the added file, change Build Action to "Content" and Copy to Output Directory to "Copy always"
1. Select "x64" from the configuration dropdown in the toolbar in Visual Studio
1. In the Project Properties, under the Debug tab, change Debugger Type for the app process to "Mixed" to be able to see asserts and debug the native Xbox Live dll
1. Change sandbox on your target device to XDKS.1.  See https://developer.microsoft.com/en-us/games/xbox/docs/xboxlive/get-started/xbox-live-sandboxes for how to change sandbox.
1. Build and run the UWP app project called "Unity (Universal Windows)" in Visual Studio
1. Sign-in to the app with an Xbox Live test account.  See https://developer.microsoft.com/en-us/games/xbox/docs/xboxlive/get-started/xbox-live-test-accounts for details on how to get one.

