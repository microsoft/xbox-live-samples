// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#include <cocos2d.h>

class IdentityLayer : public cocos2d::Layer
{
public:
    // implement the "static create()" method manually
    CREATE_FUNC(IdentityLayer);

    virtual bool init();

    virtual void onEnter();
    virtual void onExit();

    virtual void update(float dt);

    void attachToScene(cocos2d::Scene* scene);
    void detachFromScene();

    void showLayer();
    void hideLayer();

    void hasTriedSilentSignIn(bool value);

private:
    cocos2d::ui::Button* m_signInButton = nullptr;
    cocos2d::ui::Button* m_signOutButton = nullptr;
    bool m_showGamerTag = false;
    cocos2d::Label* m_gamerTagLabel = nullptr;
    bool m_userTriedSilentSignIn = false;

    void SignInButtonCallback(cocos2d::Ref* pSender, cocos2d::ui::Widget::TouchEventType type);
    void SignOutButtonCallback(cocos2d::Ref* pSender, cocos2d::ui::Widget::TouchEventType type);
};