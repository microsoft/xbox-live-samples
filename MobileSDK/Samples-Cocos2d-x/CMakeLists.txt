cmake_minimum_required(VERSION 3.6.0)

set(APP_NAME Sample)

project(${APP_NAME})

set(COCOS2DX_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../Kits/cocos2d-x)
set(CMAKE_MODULE_PATH ${COCOS2DX_ROOT_PATH}/cmake/Modules/) # needed for CocosBuldSet

include(CocosBuildSet)
add_subdirectory(${COCOS2DX_ROOT_PATH}/cocos ${ENGINE_BINARY_PATH}/cocos/core)

# record sources, headers, resources...
set(GAME_SOURCE)
set(GAME_HEADER)

set(GAME_RES_FOLDER
    "${CMAKE_CURRENT_SOURCE_DIR}/Resources"
    )
if(APPLE)
    cocos_mark_multi_resources(common_res_files RES_TO "Resources" FOLDERS ${GAME_RES_FOLDER})
endif()

if(ANDROID)
    set(ANDROID_MAVEN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../XboxLiveSDK/Android/Maven)

    # change APP_NAME to the share library name for Android, it's value depend on AndroidManifest.xml
    # set(APP_NAME MyGame)
    list(APPEND GAME_HEADER
         Sample-Android/app/jni/pch.h
         Sample-Android/app/jni/Game_Integration.h
         )
    list(APPEND GAME_SOURCE
         Sample-Android/app/jni/main.cpp
         Sample-Android/app/jni/Game_Integration.cpp
         )
elseif(APPLE)
    if(IOS)
        list(APPEND GAME_HEADER
             Sample-iOS_Mac/ios/AppController.h
             Sample-iOS_Mac/ios/RootViewController.h
             )
        set(APP_UI_RES
            Sample-iOS_Mac/ios/LaunchScreen.storyboard
            Sample-iOS_Mac/ios/LaunchScreenBackground.png
            Sample-iOS_Mac/ios/Images.xcassets
            )
        list(APPEND GAME_SOURCE
             Sample-iOS_Mac/ios/main.m
             Sample-iOS_Mac/ios/AppController.mm
             Sample-iOS_Mac/ios/RootViewController.mm
             Sample-iOS_Mac/ios/Prefix.pch
             ${APP_UI_RES}
             )
    elseif(MACOSX)
        set(APP_UI_RES
            Sample-iOS_Mac/mac/Icon.icns
            Sample-iOS_Mac/mac/Info.plist
            )
        list(APPEND GAME_SOURCE
             Sample-iOS_Mac/mac/main.cpp
             Sample-iOS_Mac/mac/Prefix.pch
             ${APP_UI_RES}
             )
    endif()
    list(APPEND GAME_SOURCE ${common_res_files})
endif()

# add cross-platforms source files and header files 
list(APPEND GAME_SOURCE
    Classes/ScreenLog.cpp
    Classes/AppDelegate.cpp
    ../Integration/XSAPI_Integration.cpp
    ../Integration/Async_Integration.cpp
    ../Integration/XAL_Integration.cpp
    ../Integration/Achievements_Integration.cpp
    Gameplay/XAL_Gameplay.cpp
    Gameplay/Achievements_Gameplay.cpp
    Scenes/GameScene.cpp
    Scenes/IdentityLayer.cpp
    Scenes/HubLayer.cpp
    Scenes/AchievementsLayer.cpp
    )
list(APPEND GAME_HEADER
    Classes/ScreenLog.h
    Classes/AppDelegate.h
    ../Integration/XSAPI_Integration.h
    ../Integration/Async_Integration.h
    ../Integration/XAL_Integration.h
    ../Integration/Achievements_Integration.h
    Scenes/GameScene.h
    Scenes/IdentityLayer.h
    Scenes/HubLayer.h
    Scenes/AchievementsLayer.h
    )

# mark app complie info and libs info
set(all_code_files
    ${GAME_HEADER}
    ${GAME_SOURCE}
    )
if(NOT ANDROID)
    add_executable(${APP_NAME} ${all_code_files})
else()
    add_library(${APP_NAME} SHARED ${all_code_files})
    add_subdirectory(${COCOS2DX_ROOT_PATH}/cocos/platform/android ${ENGINE_BINARY_PATH}/cocos/platform)
    target_link_libraries(${APP_NAME} -Wl,--whole-archive cpp_android_spec -Wl,--no-whole-archive)

    target_compile_definitions(${APP_NAME} PUBLIC
                               XSAPI_C=1
                               XSAPI_A=1
                               )

    if (${ANDROID_ABI} STREQUAL "x86")
        set(XBLSDK_ABI "x86")
        set(XALSUFFIX "AI32")
    elseif (${ANDROID_ABI} STREQUAL "armeabi-v7a")
        set(XBLSDK_ABI "arm")
        set(XALSUFFIX "AA32")
    endif()

    # Link Additional Dependencies. Note: Order matters here!
    target_link_libraries(${APP_NAME}
                          ${ANDROID_MAVEN_PATH}/ndk/libs/${XBLSDK_ABI}/libMicrosoft_Xbox_Services_Android.a
                          ${ANDROID_MAVEN_PATH}/ndk/libs/${XBLSDK_ABI}/Xal.Android-Rel${XALSUFFIX}.a
                          ${ANDROID_MAVEN_PATH}/ndk/libs/${XBLSDK_ABI}/CompactCoreCLL.Android-Rel${XALSUFFIX}.a
                          ${ANDROID_MAVEN_PATH}/ndk/libs/${XBLSDK_ABI}/liblibHttpClient_141_Android_C.a
                          ${ANDROID_MAVEN_PATH}/ndk/libs/${XBLSDK_ABI}/libssl.141.Android.a
                          ${ANDROID_MAVEN_PATH}/ndk/libs/${XBLSDK_ABI}/libcrypto.141.Android.a
                          )

    # Add Additional Include Directories
    target_include_directories(${APP_NAME}
                               PUBLIC ${ANDROID_MAVEN_PATH}/ndk/include/
                               )

    # Add Project Include Directories
    target_include_directories(${APP_NAME}
                               PUBLIC Sample-Android/app/jni/
                               )
endif()

# Add CrossPlat Project Include Directories
target_include_directories(${APP_NAME}
                           PUBLIC Classes
                           PUBLIC ../Integration
                           PUBLIC Gameplay
                           PUBLIC Scenes
                           )

target_link_libraries(${APP_NAME} cocos2d)
target_include_directories(${APP_NAME}
                           PRIVATE ${COCOS2DX_ROOT_PATH}/cocos/audio/include/
                           )

# mark app resources
setup_cocos_app_config(${APP_NAME})
if(APPLE)
    set_target_properties(${APP_NAME} PROPERTIES RESOURCE "${APP_UI_RES}")
    if(MACOSX)
        set_target_properties(${APP_NAME} PROPERTIES
                              MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/proj.ios_mac/mac/Info.plist"
                              )
    elseif(IOS)
        cocos_pak_xcode(${APP_NAME} INFO_PLIST "iOSBundleInfo.plist.in")
        set_xcode_property(${APP_NAME} ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon")
    endif()
endif()
