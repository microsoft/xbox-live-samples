// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <pch.h>
#include <XSAPI_Integration.h>
#include <Game_Integration_JNI.h>
#include "MenuView_JNI.h"

static jobject g_menuViewInstance = nullptr;
static jmethodID g_changeLayer = nullptr;

extern "C"
{
    JNIEXPORT void JNICALL Java_xbl_sample_android_views_MenuView_InitializeNativeVars(
            JNIEnv *env,
            jobject instance)
    {
        g_menuViewInstance = env->NewGlobalRef(instance);

        jclass menuView = env->GetObjectClass(instance);

        g_changeLayer = env->GetMethodID(
                menuView,
                "changeLayer",
                "(I)V");
        ASSERT_MESSAGE(g_changeLayer != nullptr, "Failed to bind Java function: changeLayer!");
    }
}

void MenuView_ChangeLayer(int layer)
{
    JNIEnv* env = Game_Integration_GetJniEnv();

    auto nextLayer = static_cast<jint>(layer);

    env->CallVoidMethod(g_menuViewInstance, g_changeLayer, nextLayer);
    ASSERT_MESSAGE(!env->ExceptionCheck(), "Failed to call 'MenuView::changeLayer'!");
}