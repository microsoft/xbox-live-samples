// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <pch.h>
#include <Game_Integration_JNI.h>
#include "ConsoleView_JNI.h"

static jobject g_consoleViewInstance = nullptr;
static jmethodID g_consoleViewLog = nullptr;

extern "C"
{
    JNIEXPORT void JNICALL Java_xbl_sample_android_views_ConsoleView_InitializeConsole(
            JNIEnv* env,
            jobject instance)
    {
        g_consoleViewInstance = env->NewGlobalRef(instance);

        jclass consoleViewClass = env->GetObjectClass(instance);

        ASSERT_MESSAGE(consoleViewClass != nullptr, "Failed to bind to Java class: 'ConsoleView'!");

        g_consoleViewLog = env->GetMethodID(
                consoleViewClass,
                "log",
                "(ILjava/lang/String;)V");
        ASSERT_MESSAGE(g_consoleViewLog != nullptr, "Failed to bind to Java method: 'ConsoleView::log'!");
    }
}

void ConsoleView_Log(int logLevel, const char* text)
{
    JNIEnv* env = Game_Integration_GetJniEnv();

    jstring consoleString = env->NewStringUTF(text);

    env->CallVoidMethod(g_consoleViewInstance, g_consoleViewLog, static_cast<jint>(logLevel), consoleString);
    ASSERT_MESSAGE(!env->ExceptionCheck(), "Failed to call 'ConsoleView::log'!");
}