// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

class Game_Integration
{
public: // Functions
    static Game_Integration* getInstance();

    bool Init(
        JNIEnv* env,
        jobject instance,
        jobject context);

    void Cleanup(
        JNIEnv* env,
        jobject instance);

    JavaVM* getJavaVM();
    JNIEnv* getJniEnv();
    jobject getAppContext();

    const std::string& getPath();

private:
    bool XsapiInit();

    XTaskQueueHandle m_asyncQueue = nullptr;

    JavaVM* m_javaVm = nullptr;
    jobject m_appContext = nullptr;
    jobject m_appActivityInstance = nullptr;
    std::string m_storagePath;
};