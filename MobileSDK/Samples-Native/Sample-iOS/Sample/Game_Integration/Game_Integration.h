// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

class Game_Integration
{
public:
    static Game_Integration* getInstance();
    
    void init();
    
    bool xboxLive_init();
    
    void setXblContext(XblContextHandle xblContext);
    XblContextHandle getXblContext();
    bool hasXblContext();
    
    XalUserHandle getCurrentUser();
    uint64_t getCurrentUserId();

private:
    XblContextHandle m_xblContext = nullptr;
    std::mutex m_xblContextMutex;
};
