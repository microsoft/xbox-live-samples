// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <pch.h>
#include "XSAPI_Integration.h"
#include "XAsync_Integration.h"

static XTaskQueueDispatchMode g_workDispatchMode = XTaskQueueDispatchMode::ThreadPool;
static XTaskQueueDispatchMode g_completionDispatchMode = XTaskQueueDispatchMode::ThreadPool;
std::thread g_backgroundThread;
std::mutex g_workReadyMutex;
bool g_workReady = false;
bool g_stopBackgroundWork = false;

#pragma region Async Integration Background Work Thread
void CALLBACK HandleXAsyncQueueCallback(_In_ void* context, _In_ XTaskQueueHandle queue, _In_ XTaskQueuePort type)
{
    UNREFERENCED_PARAMETER(context);
    UNREFERENCED_PARAMETER(queue);

    if (type == XTaskQueuePort::Work)
    {
        if (!g_workReady)
        {
            std::unique_lock<std::mutex> lock(g_workReadyMutex);
            g_workReady = true;
        }
    }
}

#if XSAPI_A
void BackgroundWorkThreadProc(_In_ XTaskQueueHandle queue,_In_ XTaskQueueRegistrationToken token, _In_ JavaVM* javaVM, _In_ JNIEnv* jniEnv)
#else
void BackgroundWorkThreadProc(_In_ XTaskQueueHandle queue,_In_ XTaskQueueRegistrationToken token)
#endif
{
    XTaskQueueHandle bgQueue = nullptr;

    XTaskQueueDuplicateHandle(queue, &bgQueue);

#if XSAPI_A
    javaVM->AttachCurrentThread(&jniEnv, nullptr);
#endif

    while (true)
    {
        if (g_stopBackgroundWork) { break; }

        if (g_workReady)
        {
            {
                std::unique_lock<std::mutex> cvLock(g_workReadyMutex);
                g_workReady = false;
            }

            bool found;

            do
            {
                if (g_stopBackgroundWork) { break; }

                found = XTaskQueueDispatch(bgQueue, XTaskQueuePort::Work, 0);
            } while(found);
        }
    }

    XTaskQueueUnregisterMonitor(bgQueue, token);
    XTaskQueueCloseHandle(bgQueue);

#if XSAPI_A
    javaVM->DetachCurrentThread();
#endif
}
#pragma endregion

#pragma region Async Integration
#if XSAPI_A
HRESULT XAsync_Init(
    _In_ XTaskQueueDispatchMode workDispatchMode,
    _In_ XTaskQueueDispatchMode completionDispatchMode,
    _Inout_ XTaskQueueHandle* asyncQueuePtr,
    _In_ JavaVM* javaVM,
    _In_ JNIEnv* jniEnv)
#else
HRESULT XAsync_Init(
    _In_ XTaskQueueDispatchMode workDispatchMode,
    _In_ XTaskQueueDispatchMode completionDispatchMode,
    _Inout_ XTaskQueueHandle* asyncQueuePtr)
#endif
{
    g_workDispatchMode = workDispatchMode;
    g_completionDispatchMode = completionDispatchMode;

    HRESULT hr = XTaskQueueCreate(
        workDispatchMode,
        completionDispatchMode,
        asyncQueuePtr);

    ASSERT_MESSAGE(SUCCEEDED(hr), "XTaskQueueCreate Failed!");

    if (g_workDispatchMode == XTaskQueueDispatchMode::Manual)
    {
        {
            std::lock_guard<std::mutex> lock(g_workReadyMutex);
            g_workReady = false;
        }

        g_stopBackgroundWork = false;

        XTaskQueueHandle asyncQueue = *asyncQueuePtr;
        XTaskQueueRegistrationToken callbackToken = {0};

        hr = XTaskQueueRegisterMonitor(asyncQueue, nullptr, HandleXAsyncQueueCallback, &callbackToken);

        ASSERT_MESSAGE(SUCCEEDED(hr), "XTaskQueueRegisterMonitor Failed!");

#if XSAPI_A
        g_backgroundThread = std::thread(BackgroundWorkThreadProc, asyncQueue, callbackToken, javaVM, jniEnv);
#else
        g_backgroundThread = std::thread(BackgroundWorkThreadProc, asyncQueue, callbackToken);
#endif

        ASSERT_MESSAGE(g_backgroundThread.get_id() != std::thread::id(), "BackgroundWorkThread Failed Thread Creation!");
    }

    return hr;
}

void XAsync_Cleanup(_In_ XTaskQueueHandle asyncQueue)
{
    if (g_workDispatchMode == XTaskQueueDispatchMode::Manual)
    {
        g_stopBackgroundWork = true;

        if (g_backgroundThread.joinable())
        {
            g_backgroundThread.join();
        }
    }

    if (asyncQueue)
    {
        XTaskQueueCloseHandle(asyncQueue);
    }
}

/// <summary>
/// Call this on the thread you want to dispatch async completions on
/// This will invoke the AsyncBlock's callback completion handler.
///
/// If there's no more completion are ready to dispatch, it will early exit and returns false
/// otherwise it will dispatch up to maxItemsToDrain number of completions and returns true
/// </summary>
bool XAsync_DrainCompletionQueue(_In_ XTaskQueueHandle queue, _In_ uint32_t maxItemsToDrain)
{
    bool found = false;

    if (g_completionDispatchMode == XTaskQueueDispatchMode::Manual)
    {
        for (uint32_t i = maxItemsToDrain; i > 0; --i)
        {
            if (g_stopBackgroundWork) { break; }

            found = XTaskQueueDispatch(queue, XTaskQueuePort::Completion, 0);
            if (!found) { break; }
        }
    }

    return found;
}

/// <summary>
/// Call this on the thread you want to dispatch async completions on
/// This will invoke the AsyncBlock's callback completion handler.
///
/// If there's no more completion are ready to dispatch, it will early exit and returns false
/// otherwise it will dispatch until at least stopAfterMilliseconds has elapsed and returns true
/// </summary>
bool XAsync_DrainCompletionQueueWithTimeout(_In_ XTaskQueueHandle queue, _In_ double stopAfterMilliseconds)
{
    bool found = false;

    if (g_completionDispatchMode == XTaskQueueDispatchMode::Manual)
    {
        std::chrono::time_point<std::chrono::system_clock> startTime;
        std::chrono::time_point<std::chrono::system_clock> endTime;
        double elapsedTime;

        do
        {
            if (g_stopBackgroundWork) { break; }

            startTime = std::chrono::system_clock::now();
            found = XTaskQueueDispatch(queue, XTaskQueuePort::Completion, 0);
            if (!found) { break; }

            endTime = std::chrono::system_clock::now();
            elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
            stopAfterMilliseconds -= elapsedTime;
            if (stopAfterMilliseconds < 0.0) { break; }

        } while (found);
    }

    return found;
}

/// <summary>
/// Call this on the thread you want to dispatch async completions on
/// This will invoke the AsyncBlock's callback completion handler.
/// </summary>
void XAsync_DrainCompletionQueueUntilEmpty(_In_ XTaskQueueHandle queue)
{
    if (g_completionDispatchMode == XTaskQueueDispatchMode::Manual)
    {
        bool found = false;

        do
        {
            if (g_stopBackgroundWork) { break; }

            found = XTaskQueueDispatch(queue, XTaskQueuePort::Completion, 0);
        } while (found);
    }
}