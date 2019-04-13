// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

class Game_Integration
{
public: // Functions
    static Game_Integration* getInstance();

    bool Init();

    void Cleanup();

private:
    bool XsapiInit();

    XTaskQueueHandle m_asyncQueue = nullptr;
};
