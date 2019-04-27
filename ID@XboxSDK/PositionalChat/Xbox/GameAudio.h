//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved
#pragma once
#include "AudioCBuffer.h"
#include <mmdeviceapi.h>
#include <xaudio2.h>
#include <x3daudio.h>
#include <xaudio2fx.h>

#define INNER_FADE                                  300
#define DISTANCE_SCALAR                             600
#define NUM_PRESETS                                 30
#define STREAMING_FRAME_SIZE                        2048

static const LPCWSTR g_File = L"Assets\\StaticLoop.wav";
static const float rotateScalar = 0.1f;

//Call back for XAudio2
//This will only be used for XAudio2 to fire an event that
//requests an updated buffer for chat audio 
struct VoiceCallback : public IXAudio2VoiceCallback
{
    HANDLE m_SampleReadyEvent;
    virtual void OnVoiceProcessingPassStart( UINT32 ) override {}
    virtual void OnVoiceProcessingPassEnd() override {}
    virtual void OnStreamEnd() override {}
    virtual void OnBufferStart( void* ) override {}
    void OnBufferEnd( void* pBufferContext )
    {
        UNREFERENCED_PARAMETER(pBufferContext);
        SetEvent( m_SampleReadyEvent );
    }
    void SetHandle( HANDLE inEvent )
    {
        m_SampleReadyEvent = inEvent;
    }
    virtual void OnLoopEnd( void* ) override {}
    virtual void OnVoiceError( void*, HRESULT ) override {}

    VoiceCallback() {}
    virtual ~VoiceCallback() {}
};

class GameAudio
{
public:
    GameAudio(){};
    void Init(int x, int y);
    void Update();
    void Update(float xdiff, float ydiff);

    void AdjustEmitterAngle(float value, float *fAngle);
    void SetReverb( int index );
    std::wstring GetReverbName(int index);

    void SetCaptureBuffer(UINT32 NumBytesGiven, BYTE *ppData)
    {
        m_chatBuffer->SetCaptureBuffer(NumBytesGiven, ppData);
    }
    
    float EmitterX()
    {
        return m_X3DEmitter.Position.x;
    }

    float EmitterY()
    {
        return m_X3DEmitter.Position.y;
    }

    float EmitterConeInner()
    {
        return m_X3DEmitter.pCone->InnerAngle;
    }

    float EmitterConeOuter()
    {
        return m_X3DEmitter.pCone->OuterAngle;
    }

private:
    IXAudio2*                       m_pXAudio2;

    IXAudio2MasteringVoice*         m_pMasteringVoice;

    IXAudio2SourceVoice*            m_pSourceVoiceChat;
    IXAudio2SourceVoice*            m_pSourceVoiceWav;

    IXAudio2SubmixVoice*            m_pSubmixVoice;

    IUnknown*                       m_pReverbEffect;

    X3DAUDIO_HANDLE			        m_X3DInstance;

    XAUDIO2_VOICE_DETAILS           m_deviceDetails;

    X3DAUDIO_DSP_SETTINGS           m_X3DDSPSettings;

    X3DAUDIO_LISTENER		        m_X3DListener;
    X3DAUDIO_EMITTER		        m_X3DEmitter;

    X3DAUDIO_DISTANCE_CURVE_POINT   VolumePoints[10];
    X3DAUDIO_DISTANCE_CURVE         VolumeCurve;
    X3DAUDIO_DISTANCE_CURVE_POINT   ReverbPoints[10];
    X3DAUDIO_DISTANCE_CURVE         ReverbCurve;

    X3DAUDIO_CONE                   EmitterCone;

    std::unique_ptr<uint8_t[]>      m_waveFile;

    VoiceCallback                   m_voiceCallback;
    HANDLE                          m_SampleReadyEvent;

    XboxSampleFramework::AudioCBuffer*   m_chatBuffer;
    std::vector<BYTE>               m_Buffer;

    HRESULT GetCaptureSample( UINT32 FramesAvailable );
    float GetDistance();
    float GetVolume(float fInner, float fOuter );
    HRESULT WriteSilence( UINT32 FramesAvailable );

    static DWORD WINAPI AudioSampleThreadProc( LPVOID lpParameter );
    DWORD AudioSampleThread();

    void PrepPCM( LPCWSTR szFilename);



};