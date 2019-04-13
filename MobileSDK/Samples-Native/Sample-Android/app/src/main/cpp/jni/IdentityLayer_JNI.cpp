// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <pch.h>
#include <XSAPI_Integration.h>
#include <Identity_Integration.h>
#include <Game_Integration.h>
#include <Game_Integration_JNI.h>
#include "IdentityLayer_JNI.h"

static jobject g_identityLayerInstance = nullptr;
static jmethodID g_setGamerPic = nullptr;
static jmethodID g_clearGamerPic = nullptr;
static jmethodID g_setGamerScore = nullptr;
static jmethodID g_clearGamerScore = nullptr;
static jmethodID g_setGamerTag = nullptr;
static jmethodID g_clearGamerTag = nullptr;
static jmethodID g_setSignInEnabled = nullptr;
static jmethodID g_setSignOutEnabled = nullptr;

extern "C"
{
    JNIEXPORT void JNICALL Java_xbl_sample_android_layers_IdentityLayer_InitializeNativeVars(
            JNIEnv* env,
            jobject instance)
    {
        g_identityLayerInstance = env->NewGlobalRef(instance);

        jclass identityLayer = env->GetObjectClass(instance);

        g_setGamerPic = env->GetMethodID(
                identityLayer,
                "setGamerPic",
                "(Ljava/lang/String;)V");
        ASSERT_MESSAGE(g_setGamerPic != nullptr, "Failed to bind Java function: setGamerPic!");

        g_clearGamerPic = env->GetMethodID(
                identityLayer,
                "clearGamerPic",
                "()V");
        ASSERT_MESSAGE(g_clearGamerPic != nullptr, "Failed to bind Java function: clearGamerPic!");

        g_setGamerScore = env->GetMethodID(
                identityLayer,
                "setGamerScore",
                "(Ljava/lang/String;)V");
        ASSERT_MESSAGE(g_setGamerScore != nullptr, "Failed to bind Java function: setGamerScore!");

        g_clearGamerScore = env->GetMethodID(
                identityLayer,
                "clearGamerScore",
                "()V");
        ASSERT_MESSAGE(g_clearGamerScore != nullptr, "Failed to bind Java function: clearGamerScore!");

        g_setGamerTag = env->GetMethodID(
                identityLayer,
                "setGamerTag",
                "(Ljava/lang/String;)V");
        ASSERT_MESSAGE(g_setGamerTag != nullptr, "Failed to bind Java function: setGamerTag!");

        g_clearGamerTag = env->GetMethodID(
                identityLayer,
                "clearGamerTag",
                "()V");
        ASSERT_MESSAGE(g_clearGamerTag != nullptr, "Failed to bind Java function: clearGamerTag!");

        g_setSignInEnabled = env->GetMethodID(
                identityLayer,
                "setSignInEnabled",
                "(Z)V");
        ASSERT_MESSAGE(g_setSignInEnabled != nullptr, "Failed to bind Java function: setSignInEnabled!");

        g_setSignOutEnabled = env->GetMethodID(
                identityLayer,
                "setSignOutEnabled",
                "(Z)V");
        ASSERT_MESSAGE(g_setSignOutEnabled != nullptr, "Failed to bind Java function: setSignOutEnabled!");
    }

    JNIEXPORT void JNICALL Java_xbl_sample_android_layers_IdentityLayer_SignInUserSilently(
            JNIEnv* env,
            jobject instance)
    {
        UNREFERENCED_PARAMETER(env);
        UNREFERENCED_PARAMETER(instance);

        HRESULT hr = Identity_TrySignInUserSilently(nullptr);

        if (FAILED(hr))
        {
            SampleLog(LL_ERROR, "XalTryAddDefaultUserSilentlyAsync Failed!");
            SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(hr).c_str());
        }
    }

    JNIEXPORT void JNICALL Java_xbl_sample_android_layers_IdentityLayer_SignInUserWithUI(
            JNIEnv* env,
            jobject instance)
    {
        UNREFERENCED_PARAMETER(env);
        UNREFERENCED_PARAMETER(instance);

        HRESULT hr = Identity_TrySignInUserWithUI(nullptr);

        if (FAILED(hr))
        {
            SampleLog(LL_ERROR, "XalAddUserWithUIAsync Failed!");
            SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(hr).c_str());
        }
    }

    JNIEXPORT void JNICALL Java_xbl_sample_android_layers_IdentityLayer_SignOutUser(
            JNIEnv* env,
            jobject instance)
    {
        UNREFERENCED_PARAMETER(env);
        UNREFERENCED_PARAMETER(instance);

        XalUserHandle xalUser = Game_Integration::getInstance()->getCurrentUser();
        HRESULT hr = Identity_TrySignOutUser(nullptr, xalUser);

        if (FAILED(hr))
        {
            SampleLog(LL_ERROR, "XalSignOutUserAsync Failed!");
            SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(hr).c_str());
        }
    }
}

void IdentityLayer_SetGamerPic(const char* gamerPic)
{
    JNIEnv* env = Game_Integration_GetJniEnv();

    jstring gamerPicString = env->NewStringUTF(gamerPic);

    env->CallVoidMethod(g_identityLayerInstance, g_setGamerPic, gamerPicString);
    ASSERT_MESSAGE(!env->ExceptionCheck(), "Failed to call 'IdentityLayer::setGamerPic'!");
}

void IdentityLayer_ClearGamerPic()
{
    JNIEnv* env = Game_Integration_GetJniEnv();

    env->CallVoidMethod(g_identityLayerInstance, g_clearGamerPic);
    ASSERT_MESSAGE(!env->ExceptionCheck(), "Failed to call 'IdentityLayer::clearGamerPic'!");
}

void IdentityLayer_SetGamerScore(const char* gamerScore)
{
    JNIEnv* env = Game_Integration_GetJniEnv();

    jstring gamerScoreString = env->NewStringUTF(gamerScore);

    env->CallVoidMethod(g_identityLayerInstance, g_setGamerScore, gamerScoreString);
    ASSERT_MESSAGE(!env->ExceptionCheck(), "Failed to call 'IdentityLayer::setGamerScore'!");
}

void IdentityLayer_ClearGamerScore()
{
    JNIEnv* env = Game_Integration_GetJniEnv();

    env->CallVoidMethod(g_identityLayerInstance, g_clearGamerScore);
    ASSERT_MESSAGE(!env->ExceptionCheck(), "Failed to call 'IdentityLayer::clearGamerScore'!");
}

void IdentityLayer_SetGamerTag(const char* gamerTag)
{
    JNIEnv* env = Game_Integration_GetJniEnv();

    jstring gamerTagString = env->NewStringUTF(gamerTag);

    env->CallVoidMethod(g_identityLayerInstance, g_setGamerTag, gamerTagString);
    ASSERT_MESSAGE(!env->ExceptionCheck(), "Failed to call 'IdentityLayer::setGamerTag'!");
}

void IdentityLayer_ClearGamerTag()
{
    JNIEnv* env = Game_Integration_GetJniEnv();

    env->CallVoidMethod(g_identityLayerInstance, g_clearGamerTag);
    ASSERT_MESSAGE(!env->ExceptionCheck(), "Failed to call 'IdentityLayer::clearGamerTag'!");
}

void IdentityLayer_SetSignInEnabled(bool value)
{
    JNIEnv* env = Game_Integration_GetJniEnv();

    env->CallVoidMethod(g_identityLayerInstance, g_setSignInEnabled, static_cast<jboolean>(value));
    ASSERT_MESSAGE(!env->ExceptionCheck(), "Failed to call 'IdentityLayer::setSignInEnabled'!");
}

void IdentityLayer_SetSignOutEnabled(bool value)
{
    JNIEnv* env = Game_Integration_GetJniEnv();

    env->CallVoidMethod(g_identityLayerInstance, g_setSignOutEnabled, static_cast<jboolean>(value));
    ASSERT_MESSAGE(!env->ExceptionCheck(), "Failed to call 'IdentityLayer::setSignOutEnabled'!");
}