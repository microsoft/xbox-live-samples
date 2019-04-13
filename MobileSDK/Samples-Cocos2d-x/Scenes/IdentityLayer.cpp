// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <pch.h>
#include <Identity_Integration.h>
#include <XSAPI_Integration.h>
#include "GameScene.h"
#include "IdentityLayer.h"

USING_NS_CC;
USING_NS_CC::ui;

#define IDENTITY_LAYER_LEVEL 1

bool IdentityLayer::init()
{
    //////////////////////////////
    // 1. super init first
    if (!Layer::init()) { return false; }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // Add Sign-In Button
    {
        m_signInButton = Button::create("images/Button.png", "images/ButtonPressed.png");

        if (m_signInButton == nullptr ||
            m_signInButton->getContentSize().width <= 0.0f ||
            m_signInButton->getContentSize().height <= 0.0f)
        {
            SampleLog(LL_WARNING, "'Sign-In' Button failed to load icons!");
        }
        else
        {
            float x = origin.x + m_signInButton->getContentSize().width/2.0f;
            float y = origin.y + visibleSize.height - m_signInButton->getContentSize().height/2.0f;
            y -= m_signInButton->getContentSize().height;
            m_signInButton->setPosition(Vec2(x,y));

            m_signInButton->addTouchEventListener(CC_CALLBACK_2(IdentityLayer::SignInButtonCallback, this));
            m_signInButton->setTitleFontName(TEXT_FONT_NAME);
            m_signInButton->setTitleText("SIGN-IN");
            m_signInButton->setTitleFontSize(16.0f);

            m_signInButton->setEnabled(false);

            this->addChild(m_signInButton);
        }
    }

    // Add Sign-Out Button
    {
        m_signOutButton = Button::create("images/Button.png", "images/ButtonPressed.png");

        if (m_signOutButton == nullptr ||
            m_signOutButton->getContentSize().width <= 0.0f ||
            m_signOutButton->getContentSize().height <= 0.0f)
        {
            SampleLog(LL_WARNING, "'Sign-Out' Button failed to load icons!");
        }
        else
        {
            float x = origin.x + visibleSize.width/2.0f;
            float y = origin.y + visibleSize.height - m_signOutButton->getContentSize().height/2.0f;
            y -= m_signOutButton->getContentSize().height;
            m_signOutButton->setPosition(Vec2(x,y));

            m_signOutButton->addTouchEventListener(CC_CALLBACK_2(IdentityLayer::SignOutButtonCallback, this));
            m_signOutButton->setTitleFontName(TEXT_FONT_NAME);
            m_signOutButton->setTitleText("SIGN-OUT");
            m_signOutButton->setTitleFontSize(16.0f);

            m_signOutButton->setEnabled(false);

            this->addChild(m_signOutButton);
        }
    }

    return true;
}

void IdentityLayer::onEnter()
{
    Layer::onEnter();

    Director::getInstance()->getScheduler()->scheduleUpdate(this, 0, false);

    SampleLog(LL_TRACE, "Entering Identity Module");

    SampleLog(LL_INFO, ""); // New Line
    SampleLog(LL_INFO, "Trying auto sign-in, please wait.");

    HRESULT hr = Identity_TrySignInUserSilently(XblGetAsyncQueue());

    if (FAILED(hr))
    {
        SampleLog(LL_ERROR, "XalTryAddDefaultUserSilentlyAsync Failed!");
        SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(hr).c_str());
    }
}

void IdentityLayer::onExit()
{
    Director::getInstance()->getScheduler()->unscheduleUpdate(this);

    SampleLog(LL_TRACE, "Leaving Identity Module");

    Layer::onExit();
}

void IdentityLayer::update(float dt)
{
    Layer::update(dt);

    if (GameScene::getInstance()->hasXblContext())
    {
        if (m_signInButton->isEnabled())
        {
            m_signInButton->setEnabled(false);
        }

        if (!m_signOutButton->isEnabled())
        {
            m_signOutButton->setEnabled(true);
        }

        if (!m_showGamerTag)
        {
            m_showGamerTag = true;

            m_gamerTagLabel = Label::createWithTTF("", TEXT_FONT_NAME, 24.0f);

            if (m_gamerTagLabel == nullptr)
            {
                SampleLog(LL_WARNING, "'Gamer Tag' Label failed to load text!");
            }
            else
            {
                auto visibleSize = Director::getInstance()->getVisibleSize();
                Vec2 origin = Director::getInstance()->getVisibleOrigin();

                float x = origin.x + visibleSize.width/2.0f;
                float y = origin.y + visibleSize.height - m_gamerTagLabel->getContentSize().height/2.0f;
                y -= 120.0f;
                m_gamerTagLabel->setPosition(Vec2(x, y));

                XblContextHandle xblContext = GameScene::getInstance()->getXblContext();
                XalUserHandle user = nullptr;
                HRESULT hr = XblContextGetUser(xblContext, &user);

                if (SUCCEEDED(hr))
                {
                    std::string gamerTag;
                    hr = Identity_GetGamerTag(user, &gamerTag);
                    if (SUCCEEDED(hr))
                    {
                        m_gamerTagLabel->setString(gamerTag);
                    }
                }

                this->addChild(m_gamerTagLabel);
            }
        }
    }
    else
    {
        if (m_userTriedSilentSignIn)
        {
            if (!m_signInButton->isEnabled())
            {
                m_signInButton->setEnabled(true);
            }
        }

        if (m_signOutButton->isEnabled())
        {
            m_signOutButton->setEnabled(false);
        }

        if (m_showGamerTag)
        {
            if (m_gamerTagLabel)
            {
                m_showGamerTag = false;
                removeChild(m_gamerTagLabel);
                m_gamerTagLabel = nullptr;
            }
        }
    }
}

void IdentityLayer::attachToScene(cocos2d::Scene* scene)
{
    if (scene)
    {
        detachFromScene();
        scene->addChild(this, IDENTITY_LAYER_LEVEL);
    }
}

void IdentityLayer::detachFromScene()
{
    if (getParent())
    {
        getParent()->removeChild(this,false);
    }
}

void IdentityLayer::showLayer()
{
    setVisible(true);
}

void IdentityLayer::hideLayer()
{
    setVisible(false);
}

void IdentityLayer::hasTriedSilentSignIn(bool value)
{
    m_userTriedSilentSignIn = value;
}

void IdentityLayer::SignInButtonCallback(cocos2d::Ref* pSender, cocos2d::ui::Widget::TouchEventType type)
{
    if (type == Widget::TouchEventType::ENDED)
    {
        if (!GameScene::getInstance()->hasXblContext())
        {
            HRESULT hr = Identity_TrySignInUserWithUI(XblGetAsyncQueue());

            if (FAILED(hr))
            {
                SampleLog(LL_ERROR, "XalAddUserWithUiAsync Failed!");
                SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(hr).c_str());
            }
        }
    }
}

void IdentityLayer::SignOutButtonCallback(cocos2d::Ref* pSender, cocos2d::ui::Widget::TouchEventType type)
{
    if (type == Widget::TouchEventType::ENDED)
    {
        if (GameScene::getInstance()->hasXblContext())
        {
            HRESULT hr = Identity_TrySignOutUser(XblGetAsyncQueue(), GameScene::getInstance()->getCurrentUser());

            if (FAILED(hr))
            {
                SampleLog(LL_ERROR, "XalSignOutUserAsync Failed!");
                SampleLog(LL_ERROR, "Error code: %s", ConvertHRtoString(hr).c_str());
            }
        }
    }
}
