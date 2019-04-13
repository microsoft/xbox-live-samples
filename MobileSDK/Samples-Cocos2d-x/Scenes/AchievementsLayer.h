// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#include <cocos2d.h>

class AchievementsLayer : public cocos2d::Layer
{
public:
    // implement the "static create()" method manually
    CREATE_FUNC(AchievementsLayer);

    virtual bool init();

    virtual void onEnter();
    virtual void onExit();

    virtual void update(float dt);

    virtual void cleanup();

    void attachToScene(cocos2d::Scene* scene);
    void detachFromScene();

    void showLayer();
    void hideLayer();

    void setHasNextResultsPage(bool value);
    bool getHasNextResultsPage();

    void setAchievementsResultHandle(XblAchievementsResultHandle resultHandle);
    XblAchievementsResultHandle getAchievementsResultHandle();

private:
    // Storing the AchievementsResultHandle if there is a next results page
    XblAchievementsResultHandle m_achievementsResultHandle = nullptr;
    std::mutex m_achievementsResultHandleMutex;
    bool m_hasNextResultsPage = false;

    cocos2d::ui::Button* m_getAchievementsForTitleButton = nullptr;
    cocos2d::ui::Button* m_getNextResultsPageButton = nullptr;
    cocos2d::ui::Button* m_getAchievementButton = nullptr;
    cocos2d::ui::Button* m_updateAchievementButton = nullptr;
    cocos2d::ui::Button* m_leaveSceneButton = nullptr;

    void GetAchievementsForTitleButtonCallback(cocos2d::Ref* pSender, cocos2d::ui::Widget::TouchEventType type);
    void GetNextResultsPageButtonCallback(cocos2d::Ref* pSender, cocos2d::ui::Widget::TouchEventType type);
    void GetAchievementButtonCallback(cocos2d::Ref* pSender, cocos2d::ui::Widget::TouchEventType type);
    void UpdateAchievementButtonCallback(cocos2d::Ref* pSender, cocos2d::ui::Widget::TouchEventType type);
    void LeaveLayerButtonCallback(cocos2d::Ref* pSender, cocos2d::ui::Widget::TouchEventType type);
};