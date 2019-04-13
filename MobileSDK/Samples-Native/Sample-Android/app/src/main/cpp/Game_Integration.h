// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

class Game_Integration
{
public: // Functions
    static Game_Integration* getInstance();

    void Init();

    void Cleanup();

    void setXblContext(XblContextHandle xblContext);
    XblContextHandle getXblContext();
    bool hasXblContext();

    XalUserHandle getCurrentUser();

private:
    HRESULT XblInit();
    HRESULT XalInit();

    XTaskQueueHandle m_asyncQueue = nullptr;

    XblContextHandle m_xblContext = nullptr;
    std::mutex m_xblContextMutex;
};