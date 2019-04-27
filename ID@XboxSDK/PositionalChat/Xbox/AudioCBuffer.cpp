//--------------------------------------------------------------------------------------
// WaveFileGenerator.cpp
//
// Demonstrates how to play an in-memory WAV file via WASAPI.
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "AudioCBuffer.h"

namespace XboxSampleFramework
{
        //
        //  AudioCBuffer()
        //
        AudioCBuffer::AudioCBuffer(UINT32 size)
        {
            ZeroMemory( &m_sourceFormat, sizeof( WAVEFORMATEX ) );
            ZeroMemory( &m_renderFormat, sizeof( WAVEFORMATEX ) );
            AllocateSize(size);
            front = 0;
            back = 0;
            m_sourceSampleSize = 0;
            m_renderSampleSize = 0;
        }


        //
        //  AudioCBuffer()
        //
        AudioCBuffer::AudioCBuffer(UINT32 size, WAVEFORMATEX* pSourceWfx, WAVEFORMATEX* pRenderWfx)
        {
            AllocateSize(size);
            front = 0;
            back = 0;
            CopyMemory(&m_sourceFormat, pSourceWfx, sizeof( WAVEFORMATEX));
            CopyMemory(&m_renderFormat, pRenderWfx, sizeof( WAVEFORMATEX));
            m_sourceSampleSize = 0;
            m_renderSampleSize = 0;

            SetFormatCalculations();
        }


        //
        //  ~AudioCBuffer()
        //
        AudioCBuffer::~AudioCBuffer()
        {
            ZeroMemory( &buffer, sizeof( size ) );
            ZeroMemory( &m_sourceFormat, sizeof( WAVEFORMATEX ) );
            ZeroMemory( &m_renderFormat, sizeof( WAVEFORMATEX ) );
        }


        //--------------------------------------------------------------------------------------
        //  Name: AllocateSize
        //  Desc: Allocates buffer
        //--------------------------------------------------------------------------------------
        void AudioCBuffer::AllocateSize(UINT32 inSize)
        {
            buffer = new BYTE[inSize];
			ZeroMemory(buffer, inSize);
            size = inSize;
            free = inSize;
        }


        //--------------------------------------------------------------------------------------
        //  Name: SetSourceFormat
        //  Desc: Sets the format of the data entering the buffering
        //--------------------------------------------------------------------------------------
        void AudioCBuffer::SetSourceFormat(WAVEFORMATEX* pSourceWfx)
        {
            CopyMemory(&m_sourceFormat, pSourceWfx, sizeof( WAVEFORMATEX));
            SetFormatCalculations();
        }

                    
        //--------------------------------------------------------------------------------------
        //  Name: SetRenderFormat
        //  Desc: Sets the format of the data stored in the buffer
        //--------------------------------------------------------------------------------------
        void AudioCBuffer::SetRenderFormat(WAVEFORMATEX* pRenderWfx)
        {
            CopyMemory(&m_renderFormat, pRenderWfx, sizeof( WAVEFORMATEX));
            SetFormatCalculations();

            //We can't trust the format in the buffer, so empty it
            front = 0;
            back = 0;
        }


        //--------------------------------------------------------------------------------------
        //  Name: SetFormatCalculations
        //  Desc: Sets the format of the data stored in the buffer
        //--------------------------------------------------------------------------------------
        void AudioCBuffer::SetFormatCalculations()
        {
			if(m_sourceFormat.cbSize != 0 && m_renderFormat.cbSize != 0)
            {
                m_sourceSampleSize = m_sourceFormat.nChannels * m_sourceFormat.nBlockAlign;
                m_renderSampleSize = m_renderFormat.nChannels * m_renderFormat.nBlockAlign;
            }
        }


        //--------------------------------------------------------------------------------------
        //  Name: GetCaptureBuffer
        //  Desc: Gets data from the buffer
        //--------------------------------------------------------------------------------------
        void AudioCBuffer::GetCaptureBuffer(UINT32 NumBytesRequested, BYTE *ppData)
        {
            //Fills the ppData buffer with data from circular buffer and silent the rest
            UINT32 BytesWritten = 0;
            
            if(NumBytesRequested > GetCurrentUsage() || m_sourceSampleSize == 0 || m_renderSampleSize == 0)
            {
                return;
            }
            
            m_CritSec.lock();
            
            if(NumBytesRequested > size - front)
            {
                //We need to circle
                CopyMemory(ppData, buffer + front, size - front);
                BytesWritten = size - front;
                front = 0;
            }

            CopyMemory(ppData + BytesWritten, buffer + front, NumBytesRequested - BytesWritten);
            front += NumBytesRequested - BytesWritten;
            free += NumBytesRequested;

            if(front >= size)
            {
                front -= size;
            }

            m_CritSec.unlock();
        }


        //--------------------------------------------------------------------------------------
        //  Name: SetCaptureBuffer
        //  Desc: Add data to buffer
        //--------------------------------------------------------------------------------------
        void AudioCBuffer::SetCaptureBuffer(UINT32 NumBytesGiven, BYTE *ppData)
        {
            if(m_sourceSampleSize == 0 || m_renderSampleSize == 0)
            {
                return;
            }
            
            m_CritSec.lock();
            UINT32 NumSamplesGiven = NumBytesGiven/(UINT32)m_sourceSampleSize;

            for( UINT32 sample = 0; sample < NumSamplesGiven; sample++ )
            {
				CopyMemory(buffer + back, ppData + (sample*m_sourceSampleSize), m_sourceSampleSize);
				back += (UINT32)m_sourceSampleSize;

				if(back >= size)
				{
					back -= size;
				}
            }

			if(m_renderSampleSize/m_sourceSampleSize*NumBytesGiven > free)
			{
				front = back;
				free = 0;
			}
			else
			{
				free -= NumBytesGiven * (UINT32)(m_renderSampleSize/m_sourceSampleSize);
			}

            m_CritSec.unlock();
        }
    }
