// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <pch.h>
#include <XSAPI_Integration.h>
#include <Achievements_Integration.h>
#include "GameScene.h"
#include "AchievementsLayer.h"

USING_NS_CC;
USING_NS_CC::ui;

#define ACHIEVEMENT_ID_1                "1"
#define SKIP_ITEMS                      0
#define MAX_ITEMS                       3

#define ACHIEVEMENTS_LAYER_LEVEL 2

bool AchievementsLayer::init()
{
    //////////////////////////////
    // 1. super init first
    if (!Layer::init()) { return false; }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // Add Get Achievements Button
    {
        m_getAchievementsForTitleButton = Button::create("images/ButtonExtended.png", "images/ButtonPressedExtended.png");

        if (m_getAchievementsForTitleButton == nullptr ||
            m_getAchievementsForTitleButton->getContentSize().width <= 0.0f ||
            m_getAchievementsForTitleButton->getContentSize().height <= 0.0f)
        {
            SampleLog(LL_WARNING, "Get Achievements Button failed to load icons!");
        }
        else
        {
            float x = origin.x + visibleSize.width/2.0f;
            float y = origin.y + visibleSize.height - m_getAchievementsForTitleButton->getContentSize().height/2.0f;
            y -= m_getAchievementsForTitleButton->getContentSize().height*3;
            m_getAchievementsForTitleButton->setPosition(Vec2(x,y));

            m_getAchievementsForTitleButton->addTouchEventListener(CC_CALLBACK_2(AchievementsLayer::GetAchievementsForTitleButtonCallback, this));
            m_getAchievementsForTitleButton->setTitleFontName(TEXT_FONT_NAME);
            m_getAchievementsForTitleButton->setTitleText("GET ACHIEVEMENTS");
            m_getAchievementsForTitleButton->setTitleFontSize(16.0f);

            this->addChild(m_getAchievementsForTitleButton);
        }
    }

    // Add Get Next Results Page Button
    {
        m_getNextResultsPageButton = Button::create("images/ButtonExtended.png", "images/ButtonPressedExtended.png");

        if (m_getNextResultsPageButton == nullptr ||
            m_getNextResultsPageButton->getContentSize().width <= 0.0f ||
            m_getNextResultsPageButton->getContentSize().height <= 0.0f)
        {
            SampleLog(LL_WARNING, "Get Next Page Button failed to load icons!");
        }
        else
        {
            float x = origin.x + visibleSize.width/2.0f;
            float y = origin.y + visibleSize.height - m_getNextResultsPageButton->getContentSize().height/2.0f;
            y -= m_getNextResultsPageButton->getContentSize().height*4;
            m_getNextResultsPageButton->setPosition(Vec2(x,y));

            m_getNextResultsPageButton->addTouchEventListener(CC_CALLBACK_2(AchievementsLayer::GetNextResultsPageButtonCallback, this));
            m_getNextResultsPageButton->setTitleFontName(TEXT_FONT_NAME);
            m_getNextResultsPageButton->setTitleText("GET NEXT PAGE");
            m_getNextResultsPageButton->setTitleFontSize(16.0f);

            m_getNextResultsPageButton->setEnabled(false);

            this->addChild(m_getNextResultsPageButton);
        }
    }

    // Add Get Achievement Button
    {
        m_getAchievementButton = Button::create("images/ButtonExtended.png", "images/ButtonPressedExtended.png");

        if (m_getAchievementButton == nullptr ||
            m_getAchievementButton->getContentSize().width <= 0.0f ||
            m_getAchievementButton->getContentSize().height <= 0.0f)
        {
            SampleLog(LL_WARNING, "Get Achievement Button failed to load icons!");
        }
        else
        {
            float x = origin.x + visibleSize.width/2.0f;
            float y = origin.y + visibleSize.height - m_getAchievementButton->getContentSize().height/2.0f;
            y -= m_getAchievementButton->getContentSize().height*5;
            m_getAchievementButton->setPosition(Vec2(x,y));

            m_getAchievementButton->addTouchEventListener(CC_CALLBACK_2(AchievementsLayer::GetAchievementButtonCallback, this));
            m_getAchievementButton->setTitleFontName(TEXT_FONT_NAME);
            m_getAchievementButton->setTitleText("GET ACHIEVEMENT 1");
            m_getAchievementButton->setTitleFontSize(16.0f);

            this->addChild(m_getAchievementButton);
        }
    }

    // Add Update Achievement Button
    {
        m_updateAchievementButton = Button::create("images/ButtonExtended.png", "images/ButtonPressedExtended.png");

        if (m_updateAchievementButton == nullptr ||
            m_updateAchievementButton->getContentSize().width <= 0.0f ||
            m_updateAchievementButton->getContentSize().height <= 0.0f)
        {
            SampleLog(LL_WARNING, "Update Achievement Button failed to load icons!");
        }
        else
        {
            float x = origin.x + visibleSize.width/2.0f;
            float y = origin.y + visibleSize.height - m_updateAchievementButton->getContentSize().height/2.0f;
            y -= m_updateAchievementButton->getContentSize().height*6;
            m_updateAchievementButton->setPosition(Vec2(x,y));

            m_updateAchievementButton->addTouchEventListener(CC_CALLBACK_2(AchievementsLayer::UpdateAchievementButtonCallback, this));
            m_updateAchievementButton->setTitleFontName(TEXT_FONT_NAME);
            m_updateAchievementButton->setTitleText("UPDATE ACHIEVEMENT 1");
            m_updateAchievementButton->setTitleFontSize(16.0f);

            this->addChild(m_updateAchievementButton);
        }
    }

    // Add Leave Module Button
    {
        m_leaveSceneButton = Button::create("images/ButtonExtended.png", "images/ButtonPressedExtended.png");

        if (m_leaveSceneButton == nullptr ||
            m_leaveSceneButton->getContentSize().width <= 0.0f ||
            m_leaveSceneButton->getContentSize().height <= 0.0f)
        {
            SampleLog(LL_WARNING, "Leave Module Button failed to load icons!");
        }
        else
        {
            float x = origin.x + visibleSize.width/2.0f;
            float y = origin.y + visibleSize.height - m_leaveSceneButton->getContentSize().height/2.0f;
            y -= m_leaveSceneButton->getContentSize().height*7;
            m_leaveSceneButton->setPosition(Vec2(x,y));

            m_leaveSceneButton->addTouchEventListener(CC_CALLBACK_2(AchievementsLayer::LeaveLayerButtonCallback, this));
            m_leaveSceneButton->setTitleFontName(TEXT_FONT_NAME);
            m_leaveSceneButton->setTitleText("MAIN MENU");
            m_leaveSceneButton->setTitleFontSize(16.0f);

            this->addChild(m_leaveSceneButton);
        }
    }

    return true;
}

void AchievementsLayer::onEnter()
{
    Layer::onEnter();

    Director::getInstance()->getScheduler()->scheduleUpdate(this, 0, false);

    SampleLog(LL_TRACE, "Entering Achievements Module");
}

void AchievementsLayer::onExit()
{
    Director::getInstance()->getScheduler()->unscheduleUpdate(this);

    SampleLog(LL_TRACE, "Leaving Achievements Module");

    Layer::onExit();
}

void AchievementsLayer::update(float dt)
{
    Layer::update(dt);

    if (!GameScene::getInstance()->hasXblContext())
    {
        hideLayer();
        detachFromScene();
        GameScene::getInstance()->m_achievementsLayer = nullptr;
    }
}

void AchievementsLayer::cleanup()
{
    if (m_achievementsResultHandle)
    {
        std::lock_guard<std::mutex> lock(m_achievementsResultHandleMutex);
        XblAchievementsResultCloseHandle(m_achievementsResultHandle);
        m_achievementsResultHandle = nullptr;
    }

    Layer::cleanup();
}

void AchievementsLayer::attachToScene(cocos2d::Scene* scene)
{
    if (scene)
    {
        detachFromScene();
        scene->addChild(this, ACHIEVEMENTS_LAYER_LEVEL);
    }
}

void AchievementsLayer::detachFromScene()
{
    if (getParent())
    {
        getParent()->removeChild(this, false);
    }
}

void AchievementsLayer::showLayer()
{
    setVisible(true);
}

void AchievementsLayer::hideLayer()
{
    setVisible(false);
}

void AchievementsLayer::setHasNextResultsPage(bool value)
{
    m_hasNextResultsPage = value;

    m_getNextResultsPageButton->setEnabled(m_hasNextResultsPage);
}

bool AchievementsLayer::getHasNextResultsPage()
{
    return m_hasNextResultsPage;
}

void AchievementsLayer::setAchievementsResultHandle(XblAchievementsResultHandle resultHandle)
{
    std::lock_guard<std::mutex> lock(m_achievementsResultHandleMutex);
    m_achievementsResultHandle = resultHandle;
}

XblAchievementsResultHandle AchievementsLayer::getAchievementsResultHandle()
{
    std::lock_guard<std::mutex> lock(m_achievementsResultHandleMutex);
    return m_achievementsResultHandle;
}

void AchievementsLayer::GetAchievementsForTitleButtonCallback(cocos2d::Ref* pSender, cocos2d::ui::Widget::TouchEventType type)
{
    if (type == Widget::TouchEventType::ENDED)
    {
        if (GameScene::getInstance()->hasXblContext())
        {
            XblContextHandle xblContext = GameScene::getInstance()->getXblContext();
            Achievements_GetAchievementsForTitle(XblGetAsyncQueue(), xblContext, SKIP_ITEMS, MAX_ITEMS);
        }
    }
}

void AchievementsLayer::GetNextResultsPageButtonCallback(cocos2d::Ref* pSender, cocos2d::ui::Widget::TouchEventType type)
{
    if (type == Widget::TouchEventType::ENDED)
    {
        if (GameScene::getInstance()->hasXblContext() && getHasNextResultsPage())
        {
            XblContextHandle xblContext = GameScene::getInstance()->getXblContext();
            XblAchievementsResultHandle resultHandle = getAchievementsResultHandle();

            Achievements_GetNextResultsPage(XblGetAsyncQueue(), xblContext, resultHandle, MAX_ITEMS);
        }
    }
}

void AchievementsLayer::GetAchievementButtonCallback(cocos2d::Ref* pSender, cocos2d::ui::Widget::TouchEventType type)
{
    if (type == Widget::TouchEventType::ENDED)
    {
        if (GameScene::getInstance()->hasXblContext())
        {
            XblContextHandle xblContext = GameScene::getInstance()->getXblContext();
            Achievements_GetAchievement(XblGetAsyncQueue(), xblContext, ACHIEVEMENT_ID_1);
        }
    }
}

void AchievementsLayer::UpdateAchievementButtonCallback(cocos2d::Ref* pSender, cocos2d::ui::Widget::TouchEventType type)
{
    if (type == Widget::TouchEventType::ENDED)
    {
        if (GameScene::getInstance()->hasXblContext())
        {
            XblContextHandle xblContext = GameScene::getInstance()->getXblContext();
            Achievements_UpdateAchievement(XblGetAsyncQueue(), xblContext, ACHIEVEMENT_ID_1, 100);
        }
    }
}

void AchievementsLayer::LeaveLayerButtonCallback(cocos2d::Ref* pSender, cocos2d::ui::Widget::TouchEventType type)
{
    if (type == Widget::TouchEventType::ENDED)
    {
        if (GameScene::getInstance()->hasXblContext())
        {
            detachFromScene();
            GameScene::getInstance()->m_achievementsLayer = nullptr;
            GameScene::getInstance()->m_hubLayer->showLayer();
        }
    }
}
