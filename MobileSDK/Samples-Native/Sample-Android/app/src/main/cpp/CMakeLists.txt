# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
set(ANDROID_MOBILE_SDK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../..)
set(ANDROID_MAVEN_PATH ${ANDROID_MOBILE_SDK_PATH}/XboxLiveSDK/Android/Maven)

set(LIB_NAME native-lib)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

set(SOURCE_FILES)

# Add Integration Files

list(APPEND SOURCE_FILES
        ${ANDROID_MOBILE_SDK_PATH}/Integration/XSAPI_Integration.cpp
        ${ANDROID_MOBILE_SDK_PATH}/Integration/XAsync_Integration.cpp
        ${ANDROID_MOBILE_SDK_PATH}/Integration/Identity_Integration.cpp
        ${ANDROID_MOBILE_SDK_PATH}/Integration/Adv_Identity_Integration.cpp
        ${ANDROID_MOBILE_SDK_PATH}/Integration/Achievements_Integration.cpp
        )

# Add Gameplay Files
list(APPEND SOURCE_FILES
        Gameplay/Identity_Gameplay.cpp
        Gameplay/Achievements_Gameplay.cpp
        )

# Add JNI Support Files
list(APPEND SOURCE_FILES
        jni/Game_Integration_JNI.cpp
        jni/ConsoleView_JNI.cpp
        jni/MenuView_JNI.cpp
        jni/IdentityLayer_JNI.cpp
        jni/AchievementsLayer_JNI.cpp
        )

# Make the Lib
add_library( # Sets the name of the library.
        ${LIB_NAME}

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        Game_Integration.cpp
        SampleLog.cpp
        ${SOURCE_FILES}
        )

target_compile_definitions(${LIB_NAME} PUBLIC
        XSAPI_C=1
        XSAPI_A=1
        )

if (${ANDROID_ABI} STREQUAL "x86")
    set(XBLSDK_ABI "x86")
    set(BUILD_ID "AI32")
elseif (${ANDROID_ABI} STREQUAL "armeabi-v7a")
    set(XBLSDK_ABI "arm")
    set(BUILD_ID "AA32")
endif()

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(BUILD_PREFIX "Dbg")
    set(BUILD_SUFFIX "D")
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set(BUILD_PREFIX "Rel")
    set(BUILD_SUFFIX "")
endif()

# Link Additional Dependencies. Note: Order matters here!
target_link_libraries(${LIB_NAME}
        ${ANDROID_MAVEN_PATH}/ndk/libs/${CMAKE_BUILD_TYPE}/${XBLSDK_ABI}/libMicrosoft_Xbox_Services_Android.a
        ${ANDROID_MAVEN_PATH}/ndk/libs/${CMAKE_BUILD_TYPE}/${XBLSDK_ABI}/Xal.Android-${BUILD_PREFIX}${BUILD_ID}${BUILD_SUFFIX}.a
        ${ANDROID_MAVEN_PATH}/ndk/libs/${CMAKE_BUILD_TYPE}/${XBLSDK_ABI}/CompactCoreCLL.Android-${BUILD_PREFIX}${BUILD_ID}${BUILD_SUFFIX}.a
        ${ANDROID_MAVEN_PATH}/ndk/libs/${CMAKE_BUILD_TYPE}/${XBLSDK_ABI}/liblibHttpClient_141_Android_C.a
        ${ANDROID_MAVEN_PATH}/ndk/libs/${CMAKE_BUILD_TYPE}/${XBLSDK_ABI}/libssl.141.Android.a
        ${ANDROID_MAVEN_PATH}/ndk/libs/${CMAKE_BUILD_TYPE}/${XBLSDK_ABI}/libcrypto.141.Android.a
        )

# Add Additional Include Directories
target_include_directories(${LIB_NAME}
        PUBLIC ${ANDROID_MAVEN_PATH}/ndk/include/
        )

# Add Project Include Directories
target_include_directories(${LIB_NAME}
        PUBLIC ${ANDROID_MOBILE_SDK_PATH}/Samples-Native/Sample-Android/app/src/main/cpp
        PUBLIC ${ANDROID_MOBILE_SDK_PATH}/Samples-Native/Sample-Android/app/src/main/cpp/jni
        PUBLIC ${ANDROID_MOBILE_SDK_PATH}/Integration/
        )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        ${LIB_NAME}

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})