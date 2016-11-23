How to run the Unity sample
---------------------------

1) Open the sample in Unity. Unity must be installed with the "Windows Store .NET Scripting backend" or "Windows Store IL2CPP Scripting backend" component
2) Open the scene Assets\Game.unity
3) Drag & drop "XboxLiveScript" from Assets window onto the Main Camera in the Hierarchy window
4) Drag & drop the \Binaries\Microsoft.Xbox.Services.winmd from the Xbox Live SDK into the assets tab
5) Build the project in Unity.
	1. Go to File | Build Settings, click Windows Store and click "Switch Platform"
	2. Click "Add Open Scenes" to add the current scene called "Game" to the build
	3. In the SDK combo box, choose "Universal 10" 
	4. In the UWP build type combo box, choose "D3D"
	5. Click the "Unity C# Projects" checkbox to generate the Assembly-Csharp.dll projects
	6. Click "Build" for Unity to generate the UWP Visual Studio project that wraps your Unity game in a UWP application. 
	   When you get prompted for a location, select the "Build" folder
6) Open Unity.sln from the \Build folder of the sample
7) Add the "Microsoft.Xbox.Live.SDK.WinRT.UWP" NuGet package references
	1. Follow "Installing NuGet packages in Xbox Live SDK targetting UWP" doc page
	2. Add NuGet package references to the the UWP app project called "Unity (Universal Windows)"
8) Add xboxservices.config to the "Unity (Universal Windows)" project
	1. Add the existing xboxservices.config from the root of the sample
	2. In the Properties window for the added file, change Build Action to "Content" and Copy to Output Directory to "Copy always"
9) Select "x86" from the configuration dropdown in the toolbar in Visual Studio
10) Change sandbox by running the script in an admin command prompt found in the Xbox Live SDK at \Tools\SwitchSandbox.cmd
	SwitchSandbox.cmd XDKS.1 
11) Build and run the UWP app project called "Unity (Universal Windows)" in Visual Studio
12) Sign-in to the app with an Xbox Live developer account

