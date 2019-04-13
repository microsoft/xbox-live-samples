// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <pch.h>
#include <XSAPI_Integration.h>
#include "GameScene.h"
#include "HubLayer.h"

USING_NS_CC;
USING_NS_CC::ui;

#define HUB_LAYER_LEVEL 1

bool HubLayer::init()
{
    //////////////////////////////
    // 1. super init first
    if (!Layer::init()) { return false; }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // Add Achievements Scene Button
    {
        m_achievemntsLayerButton = Button::create("images/ButtonExtended.png", "images/ButtonPressedExtended.png");

        if (m_achievemntsLayerButton == nullptr ||
            m_achievemntsLayerButton->getContentSize().width <= 0.0f ||
            m_achievemntsLayerButton->getContentSize().height <= 0.0f)
        {
            SampleLog(LL_WARNING, "'Achievements' Module Button failed to load icons!");
        }
        else
        {
            float x = origin.x + visibleSize.width/2.0f;
            float y = origin.y + visibleSize.height - m_achievemntsLayerButton->getContentSize().height/2.0f;
            y -= m_achievemntsLayerButton->getContentSize().height*3;
            m_achievemntsLayerButton->setPosition(Vec2(x,y));

            m_achievemntsLayerButton->addTouchEventListener(CC_CALLBACK_2(HubLayer::AchievementsLayerButtonCallback, this));
            m_achievemntsLayerButton->setTitleFontName(TEXT_FONT_NAME);
            m_achievemntsLayerButton->setTitleText("ACHIEVEMENTS");
            m_achievemntsLayerButton->setTitleFontSize(16.0f);

            this->addChild(m_achievemntsLayerButton);
        }
    }

    hideLayer();

    return true;
}

void HubLayer::onEnter()
{
    Layer::onEnter();

    Director::getInstance()->getScheduler()->scheduleUpdate(this, 0, false);

    SampleLog(LL_TRACE, "Entering Module Hub");
}

void HubLayer::onExit()
{
    Director::getInstance()->getScheduler()->unscheduleUpdate(this);

    SampleLog(LL_TRACE, "Leaving Module Hub");

    Layer::onExit();
}

void HubLayer::update(float dt)
{
    Layer::update(dt);
}

void HubLayer::attachToScene(cocos2d::Scene* scene)
{
    if (scene)
    {
        detachFromScene();
        scene->addChild(this, HUB_LAYER_LEVEL);
    }
}

void HubLayer::detachFromScene()
{
    if (getParent())
    {
        getParent()->removeChild(this,false);
    }
}

void HubLayer::showLayer()
{
    setVisible(true);
}

void HubLayer::hideLayer()
{
    setVisible(false);
}

void HubLayer::AchievementsLayerButtonCallback(cocos2d::Ref* pSender, cocos2d::ui::Widget::TouchEventType type)
{
    if (type == Widget::TouchEventType::ENDED)
    {
        if (GameScene::getInstance()->hasXblContext())
        {
            hideLayer();
            GameScene::getInstance()->m_achievementsLayer = AchievementsLayer::create();
            GameScene::getInstance()->m_achievementsLayer->attachToScene(GameScene::getInstance());
        }
    }
}