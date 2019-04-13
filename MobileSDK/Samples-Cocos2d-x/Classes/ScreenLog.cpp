// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "cocos2d.h"
#include "ScreenLog.h"

using namespace std;
using namespace cocos2d;

#define SCREENLOG_NUM_LINES             20                          // The total number of lines shown (font height will be the screen height divided by this)
#define SCREENLOG_START_WIDTH_INDENT    5.0f
#define SCREENLOG_START_HEIGHT_INDENT   5.0f
#define SCREENLOG_PRINT_BUFFER_SIZE     8192                        // The maximum total length of one log message.
#define SCREENLOG_LAYER_LEVEL           1000                        // The child level of this layer in the scene. Make it higher than your other layers, if you want to see the log :)

ScreenLog* g_screenLog = nullptr;
char g_screenLogPrintBuffer[SCREENLOG_PRINT_BUFFER_SIZE];

float getTimeMillis()
{
    timeval time;
    gettimeofday(&time, nullptr);
    unsigned long millisecs = (time.tv_sec * 1000) + (time.tv_usec/1000);
    return (float) millisecs;
}

ScreenLog::ScreenLog()
{
    pthread_mutexattr_t Attr;
    pthread_mutexattr_init(&Attr);
    pthread_mutexattr_settype(&Attr, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(&m_contentMutex, &Attr);

    m_timeout = 5000;//ms
    m_level = 0;
}

ScreenLog::~ScreenLog()
{
    {
        ScopeLock lock(&m_contentMutex);

        for (unsigned int i = 0; i < m_messages.size(); ++i)
        {
            delete m_messages[i];
        }
    }

    pthread_mutex_destroy(&m_contentMutex);
}

void ScreenLog::onEnter()
{
    Layer::onEnter();

    Director::getInstance()->getScheduler()->scheduleUpdate(this, 10000, false);
}

void ScreenLog::onExit()
{
    Director::getInstance()->getScheduler()->unscheduleUpdate(this);

    Layer::onExit();
}

void ScreenLog::setFontFile(string file)
{
    m_fontFile = file;
}

void ScreenLog::setFontSize(float size)
{
    m_fontSize = size;
}

void ScreenLog::setLevelMask(int p_level)
{
    m_level = p_level;
}

void ScreenLog::setTimeoutSeconds(float t)
{
    m_timeout = t * 1000;
}

void ScreenLog::attachToScene(cocos2d::Scene* scene)
{
    if (scene)
    {
        detachFromScene();
        scene->addChild(this, SCREENLOG_LAYER_LEVEL);
    }
}

void ScreenLog::detachFromScene()
{
    if (getParent())
    {
        getParent()->removeChild(this,false);
    }
}

ScreenLogMessage* ScreenLog::log(int p_level, const char *p_str, ...)
{
    if (! p_str ) { return nullptr; }

    if (!(p_level & m_level)) { return nullptr; }

    va_list t_va;
    va_start (t_va, p_str);
    ScreenLogMessage* slm = log(p_level, p_str, t_va);
    va_end (t_va);

    return slm;
}

ScreenLogMessage* ScreenLog::log(int p_level, const char *p_str, va_list list)
{
    if (! p_str ) { return nullptr; }

    if (!(p_level & m_level)) { return nullptr; }

    ScopeLock lock(&m_contentMutex);
    vsnprintf(g_screenLogPrintBuffer, SCREENLOG_PRINT_BUFFER_SIZE - 1, p_str, list);

    ScreenLogMessage *slm = new ScreenLogMessage(this, m_fontSize);
    slm->m_level = p_level;
    slm->m_text = g_screenLogPrintBuffer;
    slm->m_timestamp = getTimeMillis();
    m_messages.push_back(slm);

    return slm;
}

void ScreenLog::setMessageText(ScreenLogMessage *slm, const char *p_str, ...)
{
    ScopeLock lock(&m_contentMutex);

    //loop through to find matching message, in case it has already gone
    bool messageStillExists = false;

    for (int i = 0; i < m_messages.size(); ++i)
    {
        if ( m_messages[i] == slm )
        {
            messageStillExists = true;
            break;
        }
    }

    if ( !messageStillExists ) { return; }

    va_list t_va;
    va_start (t_va, p_str);
    vsnprintf(g_screenLogPrintBuffer, SCREENLOG_PRINT_BUFFER_SIZE - 1, p_str, t_va);
    va_end (t_va);

    slm->setLabelText(std::string(g_screenLogPrintBuffer));
    slm->m_timestamp = getTimeMillis();
}

void ScreenLog::update(float dt)
{
    ScopeLock lock(&m_contentMutex);

    for (int i = 0; i < m_messages.size(); ++i)
    {
        ScreenLogMessage* slm = m_messages[i];

        if ( slm->checkLabel() )
        {
            moveLabelsUp(i);
        }
    }

    float now = getTimeMillis();
    int c = 0;

    for (int i = m_messages.size()-1; i >= 0; --i)
    {
        ScreenLogMessage *slm = m_messages[i];

        if (now - slm->m_timestamp > m_timeout || c > SCREENLOG_NUM_LINES)
        {
            removeChild(slm->m_label,true);
            delete slm;
            m_messages.erase( m_messages.begin() + i );
        }

        c++;
    }
}

void ScreenLog::moveLabelsUp(int maxIndex)
{
    ScopeLock lock(&m_contentMutex);

    float fontSize;

    if (m_fontSize > 0.0f)
    {
        fontSize = m_fontSize;
    }
    else
    {
        float screenHeightPixels = Director::getInstance()->getWinSize().height;
        fontSize =  screenHeightPixels / (float)SCREENLOG_NUM_LINES - 1.0f;
    }

    if ( maxIndex >= m_messages.size() )
    {
        maxIndex = m_messages.size();
    }

    for (int i = 0; i < maxIndex; ++i)
    {
        ScreenLogMessage* slm = m_messages[i];
        Point p = slm->m_label->getPosition();
        p.y += fontSize;
        slm->m_label->setPosition( p );
    }
}

void ScreenLog::clearEntries()
{
    ScopeLock lock(&m_contentMutex);

    for (unsigned int i = 0; i < m_messages.size(); ++i)
    {
        delete m_messages[i];
    }

    m_messages.clear();
}

void ScreenLogMessage::setLabelText(string msg)
{
    // can be called from other threads, delay label creation to main thread to make sure OpenGL works
    ScopeLock lock(&m_layer->m_contentMutex);

    m_text = msg;
    m_dirty = true;
}

void ScreenLogMessage::createLabel()
{
    float fontSize;

    if (m_fontSize > 0.0f)
    {
        fontSize = m_fontSize;
    }
    else
    {
        float screenHeightPixels = Director::getInstance()->getWinSize().height;
        fontSize =  screenHeightPixels / (float)SCREENLOG_NUM_LINES - 1.0f;
    }

    m_label = Label::createWithTTF(m_text, m_layer->m_fontFile, fontSize);
    m_label->setAnchorPoint(Point(0,0));

    switch ( m_level )
    {
        case LL_TRACE:   m_label->setColor(Color3B::GRAY); break;
        case LL_DEBUG:   m_label->setColor(Color3B::GREEN); break;
        case LL_INFO:    m_label->setColor(Color3B::WHITE); break;
        case LL_WARNING: m_label->setColor(Color3B::ORANGE); break;
        default:         m_label->setColor(Color3B::RED); break;
    }

    m_label->setPosition( SCREENLOG_START_WIDTH_INDENT , SCREENLOG_START_HEIGHT_INDENT);
    m_layer->addChild(m_label);
}

//returns true if label was created for the first time (other labels should be moved upward)
bool ScreenLogMessage::checkLabel()
{
    if ( !m_label )
    {
        createLabel();
        m_dirty = false;
        return true;
    }

    if (m_dirty)
    {
        Point originalPos = m_label->getPosition();
        m_layer->removeChild(m_label,true);
        m_label = nullptr;
        createLabel();
        m_label->setPosition(originalPos);
        m_dirty = false;
    }

    return false;
}

void SampleLog(int logLevel, const char* text, ...)
{
    if (text == nullptr) { return; }

    va_list t_va;
    va_start (t_va, text);
    g_screenLog->log(logLevel, text, t_va);
    va_end (t_va);
}
