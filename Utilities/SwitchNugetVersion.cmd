@echo off
if "%1" EQU "" goto help

if EXIST %1\*.nupkg goto :extractVersion
set NEW_VERSION=%1
echo NEW_VERSION=%NEW_VERSION% 
goto :start

:extractVersion
echo Extracting version from %1\*.nupkg
dir /b %1\Microsoft.Xbox.Live.SDK.Cpp.XboxOneXDK.*.nupkg > %ROOT_FOLDER%\temp.txt
set /P NEW_VER= < %ROOT_FOLDER%\temp.txt
del %ROOT_FOLDER%\temp.txt
set NEW_VERSION=%NEW_VER:~39,-6%
echo NEW_VERSION=%NEW_VERSION% 
goto :start

:start
set ROOT_FOLDER=%~dp0..
rem findstr /C:"\"Microsoft.Xbox.Live.SDK.Cpp.UWP\"" %ROOT_FOLDER%\Samples\ID@XboxSDK\Social\UWP\Cpp\packages.config > %ROOT_FOLDER%\temp.txt
findstr /C:"\"Microsoft.Xbox.Live.SDK.Cpp.XboxOneXDK\"" %ROOT_FOLDER%\Samples\ID@XboxSDK\Social\Xbox\Cpp\packages.config > %ROOT_FOLDER%\temp.txt
set /P OLD_VER= < %ROOT_FOLDER%\temp.txt
del %ROOT_FOLDER%\temp.txt
rem set OLD_VERSION=%OLD_VER:~57,-29%
set OLD_VERSION=%OLD_VER:~64,-29%
echo OLD_VERSION=%OLD_VERSION% 
pause

cd /d %ROOT_FOLDER%\ID@XboxSDK
call :treeProcess %OLD_VERSION% %NEW_VERSION%
cd /d %ROOT_FOLDER%\CreatorsSDK
call :treeProcess %OLD_VERSION% %NEW_VERSION%

goto :eof

:treeProcess
for /f %%A IN ('dir /b *.vcxproj') do %ROOT_FOLDER%\Utilities\FindAndReplace.exe %%~fA "%1" "%2"
for /f %%A IN ('dir /b *.config') do %ROOT_FOLDER%\Utilities\FindAndReplace.exe %%~fA "%1" "%2"

for /D %%d in (*) do (
    cd %%d
    call :treeProcess %1 %2
    cd ..
)

goto done
:help

@echo off
echo SwitchNugetVersion [new version]
echo or
echo SwitchNugetVersion [build share]
echo.
echo For example:
echo SwitchNugetVersion 2017.04.20170331.003
echo or 
echo SwitchNugetVersion \\AZPKGESRLS01\XboxLiveSDK_XSAPI_Full_Build$\1806.23001\NuGetBinaries\Release\x64
:done
