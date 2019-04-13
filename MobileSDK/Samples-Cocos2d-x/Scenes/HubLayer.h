// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#include <cocos2d.h>

class HubLayer : public cocos2d::Layer
{
public:
    // implement the "static create()" method manually
    CREATE_FUNC(HubLayer);

    virtual bool init();

    virtual void onEnter();
    virtual void onExit();

    virtual void update(float dt);

    void attachToScene(cocos2d::Scene* scene);
    void detachFromScene();

    void showLayer();
    void hideLayer();

private:
    cocos2d::ui::Button* m_achievemntsLayerButton = nullptr;

    void AchievementsLayerButtonCallback(cocos2d::Ref* pSender, cocos2d::ui::Widget::TouchEventType type);
};