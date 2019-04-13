// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <pch.h>
#include "Game_Integration.h"
#include "Game_Integration_JNI.h"

static JavaVM* g_javaVm = nullptr;
static jobject g_appActivityInstance = nullptr;
static std::string g_storagePath;

// Java functions generated from AppActivity by JNI
extern "C"
{
    JNIEXPORT void JNICALL Java_xbl_sample_android_MainActivity_InitializeGame(
            JNIEnv* env,
            jobject instance)
    {
        jint j_result = env->GetJavaVM(&g_javaVm);
        ASSERT_MESSAGE(j_result == JNI_OK, "Failed to retrieve the JavaVM from provided Environment!");
        ASSERT_MESSAGE(g_javaVm != nullptr, "JavaVM is NULL!");

        g_appActivityInstance = env->NewGlobalRef(instance);

        jclass appActivity = env->GetObjectClass(instance);

        jmethodID getLocalStoragePathMethod = env->GetMethodID(
                appActivity,
                "GetLocalStoragePath",
                "()Ljava/lang/String;");
        ASSERT_MESSAGE(getLocalStoragePathMethod != nullptr, "Failed to bind Java function: GetLocalStoragePath!");

        auto path = (jstring)env->CallObjectMethod(g_appActivityInstance, getLocalStoragePathMethod);
        ASSERT_MESSAGE(!env->ExceptionCheck(), "Failed to retrieve the path to app storage!");

        char const* basePath = env->GetStringUTFChars(path, nullptr);

        g_storagePath = basePath;

        env->ReleaseStringUTFChars(path, basePath);

        Game_Integration::getInstance()->Init();
    }

    JNIEXPORT void JNICALL Java_xbl_sample_android_MainActivity_CleanupGame(
            JNIEnv* env,
            jobject instance)
    {
        UNREFERENCED_PARAMETER(env);
        UNREFERENCED_PARAMETER(instance);

        Game_Integration::getInstance()->Cleanup();
    }
}

JavaVM* Game_Integration_GetJavaVM()
{
    ASSERT_MESSAGE(g_javaVm != nullptr, "JavaVM is NULL! Must init JavaVM before grabbing JavaVM!");

    return g_javaVm;
}

JNIEnv* Game_Integration_GetJniEnv()
{
    JNIEnv* jniEnv = nullptr;
    jint j_result = Game_Integration_GetJavaVM()->GetEnv(reinterpret_cast<void**>(&jniEnv), JNI_VERSION_1_6);
    ASSERT_MESSAGE(j_result == JNI_OK, "Failed to retrieve the JNIEnv from the JavaVM!");
    ASSERT_MESSAGE(jniEnv != nullptr, "JniEnv is NULL!");

    return jniEnv;
}

jobject Game_Integration_GetAppActivityIntance()
{
    return g_appActivityInstance;
}

const std::string& Game_Integration_GetPath()
{
    return g_storagePath;
}