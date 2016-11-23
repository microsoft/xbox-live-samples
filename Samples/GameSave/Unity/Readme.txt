How to run the Unity sample
---------------------------

1) Install the Xbox Live Platform Extensions SDK from https://developer.xboxlive.com/en-us/live/development/Pages/Downloads.aspx
2) Open the sample in Unity. Unity must be installed with the "Windows Store .NET Scripting backend" or "Windows Store IL2CPP Scripting backend" component
3) Open the scene Assets\Game.unity
4) Drag & drop "XboxLiveScript" from Assets window onto the Main Camera in the Hierarchy window
5) Drag & drop \Binaries\Microsoft.Xbox.Services.winmd from the Xbox Live SDK into the assets tab
6) If your Unity version is earlier than 5.3.6p4 or 5.4.0p4:
   Drag & drop %ProgramFiles(x86)%\Windows Kits\10\References\Windows.Gaming.XboxLive.StorageApiContract\1.0.0.0\Windows.Gaming.XboxLive.StorageApiContract.winmd into the assets tab
7) Build the project in Unity.
	1. Go to File | Build Settings, click Windows Store and click "Switch Platform"
	2. Click "Add Open Scenes" to add the current scene called "Game" to the build
	3. In the SDK combo box, choose "Universal 10" 
	4. In the UWP build type combo box, choose "D3D"
	5. If your Unity version is earlier than 5.3.6p4 or 5.4.0p4:
           Click the "Unity C# Projects" checkbox to generate the Assembly-Csharp.dll projects
	6. Click "Build" for Unity to generate the UWP Visual Studio project that wraps your Unity game in a UWP application. 
	   When you get prompted for a location, select the "Build" folder
8) Open Unity.sln from the \Build folder of the sample
9) Add the "Microsoft.Xbox.Live.SDK.WinRT.UWP" NuGet package references
	1. Follow "Installing NuGet packages in Xbox Live SDK targetting UWP" doc page
	2. Add NuGet package references to the the UWP app project called "Unity (Universal Windows)"
10) If your Unity version is earlier than 5.3.6p4 or 5.4.0p4:
    If the "Unity (Universal Windows)" project contains a reference to Windows.Gaming.XboxLive.StorageApiContract, remove it to avoid a build error
11) Add xboxservices.config to the "Unity (Universal Windows)" project
	1. Add the existing xboxservices.config from the root of the sample
	2. In the Properties window for the added file, change Build Action to "Content" and Copy to Output Directory to "Copy always"
12) Select "x86" from the configuration dropdown in the toolbar in Visual Studio
13) Change sandbox by running the script in an admin command prompt found in the Xbox Live SDK at \Tools\SwitchSandbox.cmd
	SwitchSandbox.cmd XDKS.1 
14) Build and run the UWP app project called "Unity (Universal Windows)" in Visual Studio
15) Sign-in to the app with an Xbox Live developer account

