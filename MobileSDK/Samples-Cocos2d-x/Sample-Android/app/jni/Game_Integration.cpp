// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <pch.h>
#include "Game_Integration.h"
#include <XSAPI_Integration.h>

// Java functions generated from AppActivity by JNI
extern "C"
{
    void Java_org_cocos2dx_Sample_AppActivity_InitializeGame(
            JNIEnv* env,
            jobject instance,
            jobject context)
    {
        Game_Integration::getInstance()->Init(env, instance, context);
    }

    void Java_org_cocos2dx_Sample_AppActivity_CleanupGame(
            JNIEnv* env,
            jobject instance)
    {
        Game_Integration::getInstance()->Cleanup(env, instance);
    }
}

static Game_Integration* g_gInt = nullptr;

Game_Integration* Game_Integration::getInstance()
{
    if(!g_gInt)
    {
        g_gInt = new (std::nothrow) Game_Integration();
    }

    return g_gInt;
}

bool Game_Integration::Init(
    JNIEnv *env,
    jobject instance,
    jobject context)
{
    jint j_result = env->GetJavaVM(&m_javaVm);
    ASSERT_MESSAGE(j_result == JNI_OK, "Failed to retrieve the JavaVM from provided Environment!");
    ASSERT_MESSAGE(m_javaVm != nullptr, "JavaVM is NULL!");

    m_appContext = env->NewGlobalRef(context);

    m_appActivityInstance = env->NewGlobalRef(instance);

    jclass appActivity = env->GetObjectClass(instance);

    jmethodID getLocalStoragePathMethod = env->GetMethodID(
        appActivity,
        "GetLocalStoragePath",
        "()Ljava/lang/String;");

    ASSERT_MESSAGE(getLocalStoragePathMethod != nullptr, "Failed to bind Java function: GetLocalStoragePath!");

    auto path = (jstring)env->CallObjectMethod(m_appActivityInstance, getLocalStoragePathMethod);

    ASSERT_MESSAGE(!env->ExceptionCheck(), "Failed to retrieve the path to app storage!");

    char const* basePath = env->GetStringUTFChars(path, nullptr);

    m_storagePath = basePath;

    env->ReleaseStringUTFChars(path, basePath);

    return XsapiInit();
}

void Game_Integration::Cleanup(JNIEnv* env, jobject instance)
{
    UNREFERENCED_PARAMETER(env);
    UNREFERENCED_PARAMETER(instance);

    XblCleanup();
}

JavaVM* Game_Integration::getJavaVM()
{
    ASSERT_MESSAGE(m_javaVm != nullptr, "JavaVM is NULL! Must init JavaVM before grabbing JavaVM!");

    return m_javaVm;
}

JNIEnv* Game_Integration::getJniEnv()
{
    ASSERT_MESSAGE(m_javaVm != nullptr, "JavaVM is NULL! Must init JavaVM before grabbing JniEnv!");

    JNIEnv* jniEnv = nullptr;
    jint j_result = m_javaVm->GetEnv(reinterpret_cast<void**>(&jniEnv), JNI_VERSION_1_6);
    ASSERT_MESSAGE(j_result == JNI_OK, "Failed to retrieve the JNIEnv from the JavaVM!");
    ASSERT_MESSAGE(jniEnv != nullptr, "JniEnv is NULL!");

    return jniEnv;
}

jobject Game_Integration::getAppContext()
{
    return m_appContext;
}

const std::string& Game_Integration::getPath()
{
    return m_storagePath;
}

bool Game_Integration::XsapiInit()
{
    HRESULT hr = E_FAIL;

    XblInitArgs args = { };
    args.scid               = "ad560100-4bff-49c2-a4d6-44c431374b88";
    args.javaVM             = getJavaVM();
    args.applicationContext = getAppContext();

    hr = XblInitialize(&args);
    ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to Init XboxLive!");
    SampleLog(LL_DEBUG, "Xbl Init successful!");

    std::string clientId = "000000004824156c";
    std::string redirUri = "ms-xal-" + clientId + "://auth";

    XalPlatformArgs xalPlatformArgs = {};
    xalPlatformArgs.redirectUri = redirUri.c_str();
    xalPlatformArgs.javaVM      = getJavaVM();
    xalPlatformArgs.appContext  = getAppContext();

    XalInitArgs xalInitArgs = {};
    xalInitArgs.clientId     = clientId.c_str();
    xalInitArgs.titleId      = 825707400;
    xalInitArgs.sandbox      = "XDKS.1";
    xalInitArgs.platformArgs = &xalPlatformArgs;

    // Call XAL_Init after XblInitialize
    hr = XAL_Init(XblGetAsyncQueue(), &xalInitArgs, nullptr, getPath().c_str());
    ASSERT_MESSAGE(SUCCEEDED(hr), "Failed to Init XAL!");
    SampleLog(LL_DEBUG, "XAL Init successful!");

    return true;
}