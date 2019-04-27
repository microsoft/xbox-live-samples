//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved
#include "pch.h"
#include "GameAudio.h"
#include "WAVFileReader.h"

XAUDIO2FX_REVERB_I3DL2_PARAMETERS g_PRESET_PARAMS[ NUM_PRESETS ] =
{
    XAUDIO2FX_I3DL2_PRESET_FOREST,
    XAUDIO2FX_I3DL2_PRESET_DEFAULT,
    XAUDIO2FX_I3DL2_PRESET_GENERIC,
    XAUDIO2FX_I3DL2_PRESET_PADDEDCELL,
    XAUDIO2FX_I3DL2_PRESET_ROOM,
    XAUDIO2FX_I3DL2_PRESET_BATHROOM,
    XAUDIO2FX_I3DL2_PRESET_LIVINGROOM,
    XAUDIO2FX_I3DL2_PRESET_STONEROOM,
    XAUDIO2FX_I3DL2_PRESET_AUDITORIUM,
    XAUDIO2FX_I3DL2_PRESET_CONCERTHALL,
    XAUDIO2FX_I3DL2_PRESET_CAVE,
    XAUDIO2FX_I3DL2_PRESET_ARENA,
    XAUDIO2FX_I3DL2_PRESET_HANGAR,
    XAUDIO2FX_I3DL2_PRESET_CARPETEDHALLWAY,
    XAUDIO2FX_I3DL2_PRESET_HALLWAY,
    XAUDIO2FX_I3DL2_PRESET_STONECORRIDOR,
    XAUDIO2FX_I3DL2_PRESET_ALLEY,
    XAUDIO2FX_I3DL2_PRESET_CITY,
    XAUDIO2FX_I3DL2_PRESET_MOUNTAINS,
    XAUDIO2FX_I3DL2_PRESET_QUARRY,
    XAUDIO2FX_I3DL2_PRESET_PLAIN,
    XAUDIO2FX_I3DL2_PRESET_PARKINGLOT,
    XAUDIO2FX_I3DL2_PRESET_SEWERPIPE,
    XAUDIO2FX_I3DL2_PRESET_UNDERWATER,
    XAUDIO2FX_I3DL2_PRESET_SMALLROOM,
    XAUDIO2FX_I3DL2_PRESET_MEDIUMROOM,
    XAUDIO2FX_I3DL2_PRESET_LARGEROOM,
    XAUDIO2FX_I3DL2_PRESET_MEDIUMHALL,
    XAUDIO2FX_I3DL2_PRESET_LARGEHALL,
    XAUDIO2FX_I3DL2_PRESET_PLATE,
};

std::wstring g_PRESET_NAMES[ NUM_PRESETS ] =
{
    L"Forest",
    L"Default",
    L"Generic",
    L"Padded cell",
    L"Room",
    L"Bathroom",
    L"Living room",
    L"Stone room",
    L"Auditorium",
    L"Concert hall",
    L"Cave",
    L"Arena",
    L"Hangar",
    L"Carpeted hallway",
    L"Hallway",
    L"Stone Corridor",
    L"Alley",
    L"City",
    L"Mountains",
    L"Quarry",
    L"Plain",
    L"Parking lot",
    L"Sewer pipe",
    L"Underwater",
    L"Small room",
    L"Medium room",
    L"Large room",
    L"Medium hall",
    L"Large hall",
    L"Plate",
};


//--------------------------------------------------------------------------------------
// Name: Init()
// Desc: Sets up audio
//--------------------------------------------------------------------------------------
void GameAudio::Init(int x, int y)
{
    DWORD dwChannelMask;

    HRESULT hr;
    hr = XAudio2Create( &m_pXAudio2, 0 );
    if (FAILED(hr))
    {
        return;
    }
    
    hr = m_pXAudio2->CreateMasteringVoice( &m_pMasteringVoice );
    if (FAILED(hr))
    {
        return;
    }

    hr = m_pMasteringVoice->GetChannelMask( &dwChannelMask );
    if (FAILED(hr))
    {
        return;
    }

    hr = X3DAudioInitialize( dwChannelMask, X3DAUDIO_SPEED_OF_SOUND, m_X3DInstance );
    if (FAILED(hr))
    {
        return;
    }

    m_pMasteringVoice->GetVoiceDetails( &m_deviceDetails );

    hr = XAudio2CreateReverb( &m_pReverbEffect, 0 );
    if (FAILED(hr))
    {
        return;
    }

    XAUDIO2_EFFECT_DESCRIPTOR effects[] = { { m_pReverbEffect, TRUE, 1 } };
    XAUDIO2_EFFECT_CHAIN effectChain = { 1, effects };

    hr = m_pXAudio2->CreateSubmixVoice( &m_pSubmixVoice, 1, m_deviceDetails.InputSampleRate, 0, 0, nullptr, &effectChain );
    if (FAILED(hr))
    {
        return;
    }

    SetReverb(0);

    ZeroMemory(&m_X3DDSPSettings, sizeof(X3DAUDIO_DSP_SETTINGS));

    //Setup DSP
    FLOAT32 * matrix = new FLOAT32[m_deviceDetails.InputChannels];
    m_X3DDSPSettings.SrcChannelCount = 1;
    m_X3DDSPSettings.DstChannelCount = m_deviceDetails.InputChannels;
    m_X3DDSPSettings.pMatrixCoefficients = matrix;

    XAUDIO2_SEND_DESCRIPTOR sendDescriptors[2];
    sendDescriptors[0].Flags = XAUDIO2_SEND_USEFILTER; // LPF direct-path
    sendDescriptors[0].pOutputVoice = m_pMasteringVoice;
    sendDescriptors[1].Flags = XAUDIO2_SEND_USEFILTER; // LPF reverb-path -- omit for better performance at the cost of less realistic occlusion
    sendDescriptors[1].pOutputVoice = m_pSubmixVoice;
    const XAUDIO2_VOICE_SENDS sendList = { 2, sendDescriptors };

    //Setup default chat audio format
    WAVEFORMATEXTENSIBLE* pbWfx = new WAVEFORMATEXTENSIBLE();
    pbWfx->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    pbWfx->Format.nSamplesPerSec = 24000;
    pbWfx->Format.wBitsPerSample = 16;
    pbWfx->Format.cbSize = 22;
    pbWfx->Format.nChannels = 1;
    pbWfx->Format.nBlockAlign = 2;
    pbWfx->Format.nAvgBytesPerSec = 48000;
    pbWfx->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
    pbWfx->Samples.wReserved = 16;
    pbWfx->Samples.wValidBitsPerSample = 16;
    pbWfx->Samples.wSamplesPerBlock = 16;
    pbWfx->dwChannelMask = 0;

    //Create the source voice
    hr = m_pXAudio2->CreateSourceVoice( &m_pSourceVoiceChat, (WAVEFORMATEX*)pbWfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &m_voiceCallback, &sendList );
    if (FAILED(hr))
    {
        return;
    }

    //Create circular buffer for chat audio
    m_chatBuffer = new XboxSampleFramework::AudioCBuffer(32768);
    m_chatBuffer->SetRenderFormat((WAVEFORMATEX*)pbWfx);
    m_chatBuffer->SetSourceFormat((WAVEFORMATEX*)pbWfx);
    m_Buffer.resize(STREAMING_FRAME_SIZE * 2);

    //Register event for chat audio
    m_SampleReadyEvent = CreateEventEx( nullptr, nullptr, 0, EVENT_ALL_ACCESS );
    m_voiceCallback.SetHandle( m_SampleReadyEvent );
    HANDLE hAudioSampleThread = CreateThread( nullptr, 0, AudioSampleThreadProc, this, 0, nullptr );
    SetThreadPriority( hAudioSampleThread, THREAD_PRIORITY_TIME_CRITICAL );
    CloseHandle( hAudioSampleThread );

    ZeroMemory(&m_X3DListener, sizeof(X3DAUDIO_LISTENER));
    ZeroMemory(&m_X3DEmitter, sizeof(X3DAUDIO_EMITTER));

    //Listener is facing the top of the screen
    m_X3DListener.OrientFront.y = -1.0f;
    m_X3DListener.OrientTop.z = 1.0f;

    //Emitter is facing the bottom of the screen
    m_X3DEmitter.OrientFront.y = 1.0f;
    m_X3DEmitter.OrientTop.z = 1.0f;

    //Audio in use is in mono
    m_X3DEmitter.ChannelCount = 1;

    //Volume attenuation curve
    VolumePoints[0].Distance = 0.0f;
    VolumePoints[0].DSPSetting = 1.0f;
    VolumePoints[1].Distance = 0.2f;
    VolumePoints[1].DSPSetting = 1.0f;
    VolumePoints[2].Distance = 0.3f;
    VolumePoints[2].DSPSetting = 0.5f;
    VolumePoints[3].Distance = 0.4f;
    VolumePoints[3].DSPSetting = 0.35f;
    VolumePoints[4].Distance = 0.5f;
    VolumePoints[4].DSPSetting = 0.23f;
    VolumePoints[5].Distance = 0.6f;
    VolumePoints[5].DSPSetting = 0.16f;
    VolumePoints[6].Distance = 0.7f;
    VolumePoints[6].DSPSetting = 0.1f;
    VolumePoints[7].Distance = 0.8f;
    VolumePoints[7].DSPSetting = 0.06f;
    VolumePoints[8].Distance = 0.9f;
    VolumePoints[8].DSPSetting = 0.04f;
    VolumePoints[9].Distance = 1.0f;
    VolumePoints[9].DSPSetting = 0.0f;
    VolumeCurve.PointCount = 10;
    VolumeCurve.pPoints = VolumePoints;

    //Reverb attenuation curve
    ReverbPoints[0].Distance = 0.0f;
    ReverbPoints[0].DSPSetting = 0.7f;
    ReverbPoints[1].Distance = 0.2f;
    ReverbPoints[1].DSPSetting = 0.78f;
    ReverbPoints[2].Distance = 0.3f;
    ReverbPoints[2].DSPSetting = 0.85f;
    ReverbPoints[3].Distance = 0.4f;
    ReverbPoints[3].DSPSetting = 1.0f;
    ReverbPoints[4].Distance = 0.5f;
    ReverbPoints[4].DSPSetting = 1.0f;
    ReverbPoints[5].Distance = 0.6f;
    ReverbPoints[5].DSPSetting = 0.6f;
    ReverbPoints[6].Distance = 0.7f;
    ReverbPoints[6].DSPSetting = 0.4f;
    ReverbPoints[7].Distance = 0.8f;
    ReverbPoints[7].DSPSetting = 0.25f;
    ReverbPoints[8].Distance = 0.9f;
    ReverbPoints[8].DSPSetting = 0.11f;
    ReverbPoints[9].Distance = 1.0f;
    ReverbPoints[9].DSPSetting = 0.0f;
    ReverbCurve.PointCount = 10;
    ReverbCurve.pPoints = ReverbPoints;

    m_X3DEmitter.pVolumeCurve = &VolumeCurve;
    m_X3DEmitter.pReverbCurve = &ReverbCurve;

    //Emitter cone
    EmitterCone.InnerAngle = X3DAUDIO_PI/2;
    EmitterCone.OuterAngle = X3DAUDIO_PI;
    EmitterCone.InnerVolume = 1.0f;
    EmitterCone.OuterVolume = 0.0f;
    EmitterCone.InnerReverb = 1.0f;
    EmitterCone.OuterReverb = 0.0f;
    m_X3DEmitter.pCone = &EmitterCone;

    //Set how much distance influences the sound
    m_X3DEmitter.CurveDistanceScaler = DISTANCE_SCALAR;

    //Start the listener and emitter in the middle of the screen
    m_X3DListener.Position = X3DAUDIO_VECTOR((float)x/2, (float)y/2, 0);
    m_X3DEmitter.Position = X3DAUDIO_VECTOR((float)x/2, (float)(y/2)+100, 0);

    //Start the audio playback
    PrepPCM( g_File );

    //Start chat voice with silence
    WriteSilence( STREAMING_FRAME_SIZE );
    m_pSourceVoiceChat->SetVolume( 1.0f );
    m_pSourceVoiceChat->Start( 0 );
}


//--------------------------------------------------------------------------------------
// Name: Update()
// Desc: Updates audio
//--------------------------------------------------------------------------------------
void GameAudio::Update()
{
    Update(0, 0);
}


//--------------------------------------------------------------------------------------
// Name: Update()
// Desc: Updates audio
//--------------------------------------------------------------------------------------
void GameAudio::Update(float xdiff, float ydiff)

{
    //Update chat audio position
    m_X3DEmitter.Position = X3DAUDIO_VECTOR(m_X3DEmitter.Position.x + xdiff, m_X3DEmitter.Position.y - ydiff, 0);
    m_X3DEmitter.Velocity = X3DAUDIO_VECTOR(xdiff, ydiff, 0);

    //Set volume for wave file (chat volume handled by 3D audio)
    float fDistance = GetDistance();
    if(fDistance < INNER_FADE)
    {
        m_pSourceVoiceWav->SetVolume( 0 );
    }
    else if(fDistance > INNER_FADE && fDistance <= DISTANCE_SCALAR)
    {
        m_pSourceVoiceWav->SetVolume(GetVolume(INNER_FADE, DISTANCE_SCALAR));
    }
    if(fDistance > DISTANCE_SCALAR)
    {
        m_pSourceVoiceWav->SetVolume( 1.0f );
    }

    //Update 3D audio
    X3DAudioCalculate( m_X3DInstance, &m_X3DListener, &m_X3DEmitter, X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER | X3DAUDIO_CALCULATE_REVERB, &m_X3DDSPSettings );
    m_pSourceVoiceChat->SetOutputMatrix( m_pMasteringVoice, 1, m_deviceDetails.InputChannels, m_X3DDSPSettings.pMatrixCoefficients ) ;
    m_pSourceVoiceChat->SetFrequencyRatio(m_X3DDSPSettings.DopplerFactor);
    m_pSourceVoiceChat->SetOutputMatrix(m_pSubmixVoice, 1, 1, &m_X3DDSPSettings.ReverbLevel);
}


//--------------------------------------------------------------------------------------
// Name: AdjustEmitterAngle()
// Desc: Adjusts the front orientation and angle of the emitter based on an input value
//--------------------------------------------------------------------------------------
void GameAudio::AdjustEmitterAngle(float value, float *fAngle)
{
    float temp = *fAngle;
    temp += value*rotateScalar;

    //Keep between 0 and 2 pi
    if(temp >= X3DAUDIO_2PI)
    {
        temp -= X3DAUDIO_2PI;
    }
    else if (temp < 0)
    {
        temp += X3DAUDIO_2PI;
    }

    //Update orientation as a unit vector based on the angle
    m_X3DEmitter.OrientFront.x = sin(temp);
    m_X3DEmitter.OrientFront.y = -cos(temp);

    CopyMemory(fAngle, &temp, sizeof(float));
}


//--------------------------------------------------------------------------------------
// Name: GetReverbName()
// Desc: Provides the string of the current reverb
//--------------------------------------------------------------------------------------
std::wstring GameAudio::GetReverbName(int index)
{
    return g_PRESET_NAMES[index];
}


//--------------------------------------------------------------------------------------
// Name: GetDistance()
// Desc: Provides the distance between the emitter and listener
//--------------------------------------------------------------------------------------
float GameAudio::GetDistance()
{
    float fXDiff = abs(m_X3DListener.Position.x - m_X3DEmitter.Position.x);
    float fYDiff = abs(m_X3DListener.Position.y - m_X3DEmitter.Position.y);
    float fDistance = sqrt(fXDiff * fXDiff + fYDiff * fYDiff);
    return fDistance;
}


//--------------------------------------------------------------------------------------
// Name: GetVolume()
// Desc: Provides a volume (0 to 1.0f)
//--------------------------------------------------------------------------------------
float GameAudio::GetVolume(float fInner, float fOuter )
{
    float fDistance = GetDistance();
    float fOutput = 0;

    if(fDistance < fInner)
    {
        fOutput = 1.0f;
    }
    else if(fDistance < fOuter)
    {
        fOutput = (fDistance - fInner) / (fOuter - fInner);
    }

    return fOutput;
}


//--------------------------------------------------------------------------------------
//  Name:   GetCaptureSample
//  Desc:   Fills buffer with a capture sample
//--------------------------------------------------------------------------------------
HRESULT GameAudio::GetCaptureSample( UINT32 FramesAvailable )
{
    HRESULT hr = S_OK;

    UINT32 BytesToUse = FramesAvailable *2;
    
    if(m_chatBuffer->GetCurrentUsage() < BytesToUse)
    {
        BytesToUse = m_chatBuffer->GetCurrentUsage();
    }

    //Chat will always be 16 bit, mono 
    XAUDIO2_BUFFER buffer = {};
    buffer.pAudioData = &m_Buffer[0];
    buffer.AudioBytes = BytesToUse;

    m_chatBuffer->GetCaptureBuffer( BytesToUse, &m_Buffer[0]);
    hr = m_pSourceVoiceChat->SubmitSourceBuffer( &buffer );

    return hr;
}


//--------------------------------------------------------------------------------------
//  Name:   WriteSilence
//  Desc:   Fills buffer with silence
//--------------------------------------------------------------------------------------
HRESULT GameAudio::WriteSilence( UINT32 FramesAvailable )
{
    HRESULT hr = S_OK;

    XAUDIO2_BUFFER buffer = {};
    buffer.pAudioData = &m_Buffer[0];
    buffer.AudioBytes = FramesAvailable * 2;

    m_Buffer.empty();

    hr = m_pSourceVoiceChat->SubmitSourceBuffer( &buffer );

    return hr;
}


//--------------------------------------------------------------------------------------
// Name: SetReverb()
// Desc: Sets the reverb settings
//--------------------------------------------------------------------------------------
void GameAudio::SetReverb( int index )
{
    XAUDIO2FX_REVERB_PARAMETERS native;
    ReverbConvertI3DL2ToNative( &g_PRESET_PARAMS[index], &native );
    //Override rear for mono
    native.RearDelay = 5;
    m_pSubmixVoice->SetEffectParameters( 0, &native, sizeof( native ) );
}


//--------------------------------------------------------------------------------------
// Name: PrepPCM
// Desc: Plays a repeating wav
//--------------------------------------------------------------------------------------
void GameAudio::PrepPCM( LPCWSTR szFilename)
{
    HRESULT hr = S_OK;
    DX::WAVData WavData;

    hr = DX::LoadWAVAudioFromFileEx(szFilename, m_waveFile, WavData);
    if (FAILED(hr))
    {
        return;
    }

    XAUDIO2_SEND_DESCRIPTOR sendDescriptors[2];
    sendDescriptors[0].Flags = XAUDIO2_SEND_USEFILTER; // LPF direct-path
    sendDescriptors[0].pOutputVoice = m_pMasteringVoice;
    sendDescriptors[1].Flags = XAUDIO2_SEND_USEFILTER; // LPF reverb-path -- omit for better performance at the cost of less realistic occlusion
    sendDescriptors[1].pOutputVoice = m_pSubmixVoice;
    const XAUDIO2_VOICE_SENDS sendList = { 2, sendDescriptors };

    // Create the source voice
    hr = m_pXAudio2->CreateSourceVoice( &m_pSourceVoiceWav, WavData.wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &m_voiceCallback, &sendList );
    if (FAILED(hr))
    {
        return;
    }

    XAUDIO2_BUFFER buffer = {};
    buffer.pAudioData = WavData.startAudio;
    buffer.AudioBytes = WavData.loopLength;
    buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
    buffer.pContext = (void*)WavData.startAudio; // Pointer to audio data in context so we can free it later
    buffer.Flags = XAUDIO2_END_OF_STREAM;  // tell the source voice not to expect any data after this buffer

    hr = m_pSourceVoiceWav->SubmitSourceBuffer( &buffer );
    if (FAILED(hr))
    {
        return;
    }

    hr = m_pSourceVoiceWav->SetVolume( 0 );
    if (FAILED(hr))
    {
        return;
    }

    hr = m_pSourceVoiceWav->Start(0);
    if (FAILED(hr))
    {
        return;
    }
}


//--------------------------------------------------------------------------------------
//  Name:   AudioSampleThreadProc
//  Desc:   Shim for the AudioSampleThread method
//--------------------------------------------------------------------------------------
DWORD WINAPI GameAudio::AudioSampleThreadProc( LPVOID lpParameter )
{
    DWORD	result = S_OK;

    result = ((GameAudio*)lpParameter)->AudioSampleThread();

    return result;
}

//--------------------------------------------------------------------------------------
//  Name:   AudioSampleThread
//  Desc:   High priority thread that services samples and commands
//--------------------------------------------------------------------------------------
DWORD GameAudio::AudioSampleThread()
{
    DWORD result = S_OK;
    bool bQuit = false;

    while ( !bQuit )
    {
        DWORD reason = WaitForSingleObjectEx( m_SampleReadyEvent, INFINITE, TRUE );

        if ( WAIT_OBJECT_0 == reason )
        {
            if(m_chatBuffer->GetCurrentUsage() == 0)
            {
                WriteSilence( STREAMING_FRAME_SIZE );
            }
            else
            {
                //Update sources
                float tempDistance = GetDistance();
                if(tempDistance < DISTANCE_SCALAR)
                {
                    GetCaptureSample( STREAMING_FRAME_SIZE );
                }
            }
        }
        else if ( WAIT_FAILED == reason )
        {
            bQuit = true;
        }
    }
    return result;
}
