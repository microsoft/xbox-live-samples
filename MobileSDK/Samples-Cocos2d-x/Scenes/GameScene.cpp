// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <pch.h>
#include <XSAPI_Integration.h>
#include "GameScene.h"

USING_NS_CC;
USING_NS_CC::ui;

static GameScene* s_gameScene = nullptr;

GameScene* GameScene::getInstance()
{
    if (!s_gameScene)
    {
        s_gameScene = GameScene::create();
    }

    return s_gameScene;
}

bool GameScene::init()
{
    //////////////////////////////
    // 1. super init first
    if (!Scene::init()) { return false; }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // Add Sample Title Label
    {
        m_sampleTitleLabel = Label::createWithTTF("XBOX LIVE SAMPLE", TEXT_FONT_NAME, 36.0f);

        if (m_sampleTitleLabel == nullptr)
        {
            SampleLog(LL_WARNING, "'Sample Title' Label failed to load text!");
        }
        else
        {
            float x = origin.x + visibleSize.width/2.0f;
            float y = origin.y + visibleSize.height - m_sampleTitleLabel->getContentSize().height/2.0f;
            m_sampleTitleLabel->setPosition(Vec2(x, y));

            this->addChild(m_sampleTitleLabel);
        }
    }

    // Add Leave Sample Button
    {
        m_leaveSampleButton = Button::create("images/Button.png", "images/ButtonPressed.png");

        if (m_leaveSampleButton == nullptr ||// m_leaveSampleButtonLabel == nullptr ||
            m_leaveSampleButton->getContentSize().width <= 0.0f ||
            m_leaveSampleButton->getContentSize().height <= 0.0f)
        {
            SampleLog(LL_WARNING, "'Exit Sample' Button failed to load icons!");
        }
        else
        {
            float x = origin.x + visibleSize.width - m_leaveSampleButton->getContentSize().width/2.0f;
            float y = origin.y + visibleSize.height - m_leaveSampleButton->getContentSize().height/2.0f;
            y -= m_leaveSampleButton->getContentSize().height;
            m_leaveSampleButton->setPosition(Vec2(x,y));

            m_leaveSampleButton->addTouchEventListener(CC_CALLBACK_2(GameScene::LeaveSampleButtonCallback, this));
            m_leaveSampleButton->setTitleFontName(TEXT_FONT_NAME);
            m_leaveSampleButton->setTitleText("EXIT SAMPLE");
            m_leaveSampleButton->setTitleFontSize(16.0f);

            this->addChild(m_leaveSampleButton);
        }
    }

    g_screenLog->attachToScene(this);

    return true;
}

void GameScene::onEnter()
{
    Scene::onEnter();

    Director::getInstance()->getScheduler()->scheduleUpdate(this, 0, false);

    SampleLog(LL_TRACE, "Entering Game Scene");

    m_hubLayer = HubLayer::create();
    m_hubLayer->attachToScene(this);

    m_identityLayer = IdentityLayer::create();
    m_identityLayer->attachToScene(this);
}

void GameScene::onExit()
{
    Director::getInstance()->getScheduler()->unscheduleUpdate(this);

    SampleLog(LL_TRACE, "Leaving Game Scene");

    Scene::onExit();
}

void GameScene::update(float dt)
{
    Scene::update(dt);
}

void GameScene::cleanup()
{
    if (m_identityLayer)
    {
        m_identityLayer->detachFromScene();
        m_identityLayer = nullptr;
    }

    if (m_hubLayer)
    {
        m_hubLayer->detachFromScene();
        m_hubLayer = nullptr;
    }

    if (m_achievementsLayer)
    {
        m_achievementsLayer->detachFromScene();
        m_achievementsLayer = nullptr;
    }

    if (m_xblContext)
    {
        XalUserHandle user = nullptr;
        HRESULT hr = XblContextGetUser(m_xblContext, &user);

        if (SUCCEEDED(hr))
        {
            XalUserCloseHandle(user);
        }

        XblContextCloseHandle(m_xblContext);
    }

    Scene::cleanup();
}

void GameScene::setXblContext(XblContextHandle xblContext)
{
    std::lock_guard<std::mutex> lock(m_xblContextMutex);
    m_xblContext = xblContext;
}

XblContextHandle GameScene::getXblContext()
{
    std::lock_guard<std::mutex> lock(m_xblContextMutex);
    return m_xblContext;
}

bool GameScene::hasXblContext()
{
    return m_xblContext != nullptr;
}

void GameScene::LeaveSampleButtonCallback(cocos2d::Ref* pSender, cocos2d::ui::Widget::TouchEventType type)
{
    switch (type)
    {
        case Widget::TouchEventType::ENDED:
        {
            cocos2d::Director::getInstance()->end();
        }   break;
        default:
            break;
    }
}

XalUserHandle GameScene::getCurrentUser()
{
    XalUserHandle user = nullptr;

    HRESULT hr = XblContextGetUser(m_xblContext, &user);

    if (FAILED(hr))
    {
        SampleLog(LL_ERROR, "XblContextGetUser Failed!");
        SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(hr).c_str());
    }

    return user;
}
