// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <pch.h>
#include <XSAPI_Integration.h>
#include <Achievements_Integration.h>
#include <Game_Integration.h>
#include <Game_Integration_JNI.h>
#include "AchievementsLayer_JNI.h"

#define ACHIEVEMENT_ID_1                "1"
#define SKIP_ITEMS                      0
#define MAX_ITEMS                       3

static jobject g_achievementsLayerInstance = nullptr;
static jmethodID g_setHasNextPage = nullptr;

static XblAchievementsResultHandle g_achievementsResultHandle = nullptr;

extern "C"
{
    JNIEXPORT void JNICALL Java_xbl_sample_android_layers_AchievementsLayer_InitializeNativeVars(
            JNIEnv* env,
            jobject instance)
    {
        g_achievementsLayerInstance = env->NewGlobalRef(instance);

        jclass achievementsLayer = env->GetObjectClass(instance);

        g_setHasNextPage = env->GetMethodID(
                achievementsLayer,
                "setHasNextPage",
                "(Z)V");

        ASSERT_MESSAGE(g_setHasNextPage != nullptr, "Failed to bind Java function: setHasNextPage!");
    }

    JNIEXPORT void JNICALL Java_xbl_sample_android_layers_AchievementsLayer_GetAchievements(
            JNIEnv* env,
            jobject instance)
    {
        UNREFERENCED_PARAMETER(env);
        UNREFERENCED_PARAMETER(instance);

        if (Game_Integration::getInstance()->hasXblContext())
        {
            XblContextHandle xblContext = Game_Integration::getInstance()->getXblContext();
            Achievements_GetAchievementsForTitle(nullptr, xblContext, SKIP_ITEMS, MAX_ITEMS);
        }
    }

    JNIEXPORT void JNICALL Java_xbl_sample_android_layers_AchievementsLayer_GetNextPage(
            JNIEnv* env,
            jobject instance)
    {
        UNREFERENCED_PARAMETER(env);
        UNREFERENCED_PARAMETER(instance);

        if (Game_Integration::getInstance()->hasXblContext())
        {
            XblContextHandle xblContext = Game_Integration::getInstance()->getXblContext();
            Achievements_GetNextResultsPage(nullptr, xblContext, g_achievementsResultHandle, MAX_ITEMS);
        }
    }

    JNIEXPORT void JNICALL Java_xbl_sample_android_layers_AchievementsLayer_GetAchievement(
            JNIEnv* env,
            jobject instance)
    {
        UNREFERENCED_PARAMETER(env);
        UNREFERENCED_PARAMETER(instance);

        if (Game_Integration::getInstance()->hasXblContext())
        {
            XblContextHandle xblContext = Game_Integration::getInstance()->getXblContext();
            Achievements_GetAchievement(nullptr, xblContext, ACHIEVEMENT_ID_1);
        }
    }

    JNIEXPORT void JNICALL Java_xbl_sample_android_layers_AchievementsLayer_UpdateAchievement(
            JNIEnv* env,
            jobject instance)
    {
        UNREFERENCED_PARAMETER(env);
        UNREFERENCED_PARAMETER(instance);

        if (Game_Integration::getInstance()->hasXblContext())
        {
            XblContextHandle xblContext = Game_Integration::getInstance()->getXblContext();
            Achievements_UpdateAchievement(nullptr, xblContext, ACHIEVEMENT_ID_1, 100);
        }
    }
}

void AchievementsLayer_SetHasNextPage(bool value)
{
    JNIEnv* env = Game_Integration_GetJniEnv();

    auto hasNextPage = static_cast<jboolean>(value);

    env->CallVoidMethod(g_achievementsLayerInstance, g_setHasNextPage, hasNextPage);

    ASSERT_MESSAGE(!env->ExceptionCheck(), "Failed to call 'AchievementsLayer::setHasNextPage'!");
}

void AchievementsLayer_SetAchievementsResultHandle(XblAchievementsResultHandle resultHandle)
{
    g_achievementsResultHandle = resultHandle;
}