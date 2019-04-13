// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#if XSAPI_A
#include <jni.h>
HRESULT XAsync_Init(
    _In_ XTaskQueueDispatchMode workDispatchMode,
    _In_ XTaskQueueDispatchMode completionDispatchMode,
    _Inout_ XTaskQueueHandle* asyncQueuePtr,
    _In_ JavaVM* javaVM,
    _In_ JNIEnv* jniEnv);
#else
HRESULT XAsync_Init(
    _In_ XTaskQueueDispatchMode workDispatchMode,
    _In_ XTaskQueueDispatchMode completionDispatchMode,
    _Inout_ XTaskQueueHandle* asyncQueuePtr);
#endif

void XAsync_Cleanup(_In_ XTaskQueueHandle asyncQueue);
bool XAsync_DrainCompletionQueue(_In_ XTaskQueueHandle queue, _In_ uint32_t maxItemsToDrain);
bool XAsync_DrainCompletionQueueWithTimeout(_In_ XTaskQueueHandle queue, _In_ double stopAfterMilliseconds);
void XAsync_DrainCompletionQueueUntilEmpty(_In_ XTaskQueueHandle queue);