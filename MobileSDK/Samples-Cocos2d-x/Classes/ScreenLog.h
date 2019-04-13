// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#include <string>
#include <vector>
#include "cocos2d.h"

#define LL_FATAL    0x01
#define LL_ERROR    0x02
#define LL_WARNING  0x04
#define LL_INFO     0x08
#define LL_DEBUG    0x10
#define LL_TRACE    0x20

class ScreenLogMessage
{
    friend class ScreenLog;

private:
    class ScreenLog* m_layer;
    float m_fontSize = 0.0f;
    int m_level;
    std::string m_text;
    float m_timestamp;
    cocos2d::Label* m_label;
    bool m_dirty;

private:
    ScreenLogMessage(ScreenLog* layer, float fontSize)
    {
        m_fontSize = fontSize;
        m_layer = layer;
        m_label = NULL;
        m_level = 0;
        m_timestamp = 0;
        m_dirty = true;
    }
    virtual ~ScreenLogMessage() { }

    void setLabelText(std::string msg);
    void createLabel();
    bool checkLabel();
};

class ScreenLog : public cocos2d::Layer
{
    friend class ScreenLogMessage;
private:
    std::string m_fontFile;
    float m_fontSize = 0.0f;
    int m_level;
    float m_timeout;
    std::vector<ScreenLogMessage*> m_messages;
    pthread_mutex_t m_contentMutex;

public:

    ScreenLog();
    ~ScreenLog();

    virtual void onEnter();
    virtual void onExit();

    void setFontFile(std::string file);
    void setFontSize(float size);
    void setLevelMask(int level);
    void setTimeoutSeconds(float seconds);

    void attachToScene(cocos2d::Scene* scene);
    void detachFromScene();

    ScreenLogMessage* log(int level, const char* msg, ...);
    ScreenLogMessage* log(int p_level, const char *p_str, va_list list);
    void setMessageText(ScreenLogMessage* slm, const char *p_str, ...);

    void update(float dt);
    void moveLabelsUp(int maxIndex);
    void clearEntries();
};

extern ScreenLog* g_screenLog;

class ScopeLock
{
public:
    pthread_mutex_t* m_mutex;

    ScopeLock(pthread_mutex_t* m)
    {
        m_mutex = m;
        pthread_mutex_lock(m_mutex);
    }

    ~ScopeLock()
    {
        pthread_mutex_unlock(m_mutex);
    }
};

class ScopeLog
{
public:
    std::string m_functionName;

    ScopeLog(std::string fn)
    {
        m_functionName = fn;
        g_screenLog->log(LL_TRACE, "Entered %s", m_functionName.c_str());
    }

    ~ScopeLog()
    {
        g_screenLog->log(LL_TRACE, "Exiting %s", m_functionName.c_str());
    }
};

void SampleLog(int logLevel, const char* text, ...);