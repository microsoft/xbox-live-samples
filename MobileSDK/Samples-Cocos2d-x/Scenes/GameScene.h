// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#define TEXT_FONT_NAME "fonts/arialbd.ttf"

#include <cocos2d.h>

// Include all Layers here
#include "IdentityLayer.h"
#include "HubLayer.h"
#include "AchievementsLayer.h"

class GameScene : public cocos2d::Scene
{
public:
    // implement the "static create()" method manually
    CREATE_FUNC(GameScene);

    static GameScene* getInstance();

    virtual bool init();

    virtual void onEnter();
    virtual void onExit();

    virtual void update(float dt);

    virtual void cleanup();

    void setXblContext(XblContextHandle xblContext);
    XblContextHandle getXblContext();
    bool hasXblContext();

    XalUserHandle getCurrentUser();

    // Stores the Layers created
    IdentityLayer* m_identityLayer = nullptr;
    HubLayer* m_hubLayer = nullptr;
    AchievementsLayer* m_achievementsLayer = nullptr;

private:
    XblContextHandle m_xblContext = nullptr;
    std::mutex m_xblContextMutex;

    cocos2d::Label* m_sampleTitleLabel = nullptr;
    cocos2d::ui::Button* m_leaveSampleButton = nullptr;

    void LeaveSampleButtonCallback(cocos2d::Ref* pSender, cocos2d::ui::Widget::TouchEventType type);
};