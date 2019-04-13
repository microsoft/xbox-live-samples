To build and run the Cocos2d-x samples, you will need to do the following setup steps:

1) Make sure you've saved your sdk.samples folder as close to your C:/ drive as possible to ensure no issues with max path length
   - Example: C:/git/sdk.samples
2) Install Python 2.7.15 and add it to your computer's environment PATH:
   - https://www.python.org/downloads/release/python-2715/
3) Ensure that the Cocos2d-x submodule has been initialized:
   - Go to your sdk.samples root folder and open the command window
   - Type in: git submodule update --init --recursive
4) Download Cocos2d-x dependencies:
   - Go to your sdk.samples/Kits/cocos2d-x folder and open the command window
   - Run "download-deps.py" from the command window to download Cocos2d-x dependencies
5) Setup Cocos2d-x environment variables:
   - Go to your sdk.samples/Kits/cocos2d-x folder and open the command window
   - Run "setup.py", this will create/update the environment variables used by Cocos2d-x build scripts
   - Ensure that all of the paths given are valid and not old paths
   - (Andriod users) Ensure the Andriod SDK and NDK paths needed by cocos2d-x link to the paths saved by Android Studio
                     (default Android Studio SDK path on windows: C:\Users\%user%\AppData\Local\Android\Sdk)
                     (default Android Studio NDK path on windows: C:\Users\%user%\AppData\Local\Android\Sdk\ndk-bundle)
                     
To build Sample-Android inside of Samples-Cocos2d-x

1) Download the XboxLiveSDK and store your Android folder into sdk.samples\MobileSDK\XboxLiveSDK
   - Follow the README.md inside the sdk.samples\MobileSDK\XboxLiveSDK folder
2) Ensure that Maven is setup:
   - Open build.gradle inside of sdk.samples\MobileSDK\Samples-Cocos2d-x\Sample-Android
   - Replace the url 'file:///C:/git/sdk.samples/MobileSDK/XboxLiveSDK/Android/Maven' to your path to Maven
3) Open Android Studio and import sdk.samples\MobileSDK\Samples-Cocos2d-x\Sample-Android
   - This will create the project implementation files, install dependencies, and link with Maven files
4) Build project:
   - Sync project with gradle files
   - Make project

To build Sample-iOS_Mac inside of Samples-Cocos2d-x

