//--------------------------------------------------------------------------------------
// WaveFileGenerator.h
//
// Demonstrates how to play an in-memory WAV file via WASAPI.
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#ifndef AudioCBuffer_H_INCLUDED
#define AudioCBuffer_H_INCLUDED

#include "pch.h"
#include <mutex>

namespace XboxSampleFramework
{
        class AudioCBuffer
        {
        public:
            AudioCBuffer(UINT32 size);
            AudioCBuffer(UINT32 size, WAVEFORMATEX* pSourceWfx, WAVEFORMATEX* pRenderWfx);
            virtual ~AudioCBuffer(void);

            void GetCaptureBuffer(UINT32 NumBytesRequested, BYTE *ppData);
            void SetCaptureBuffer(UINT32 NumBytesGiven, BYTE *ppData);

            void SetSourceFormat(WAVEFORMATEX* pSourceWfx);
            void SetRenderFormat(WAVEFORMATEX* pRenderWfx);

            const UINT32 GetCurrentUsage(){ return size - free;};

        private:
            void AllocateSize(UINT32 inSize);
            void AudioCBuffer::SetFormatCalculations();

            WAVEFORMATEX          m_sourceFormat;
            WAVEFORMATEX          m_renderFormat;
            BYTE*				buffer;
            UINT32				size;
            UINT32				front;
            UINT32				back;
            UINT32				free;
            std::mutex          m_CritSec;
            size_t				m_sourceSampleSize;
            size_t				m_renderSampleSize;
        };
    }

#endif