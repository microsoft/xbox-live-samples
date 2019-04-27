//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved
#pragma once

#include <Windows.Storage.Streams.h>
#include <wrl.h>
#include <robuffer.h>

using Microsoft::WRL::FtmBase;
using Microsoft::WRL::RuntimeClass;
using Microsoft::WRL::RuntimeClassFlags;
using Microsoft::WRL::WinRtClassicComMix;

class WrapBuffer 
    : public RuntimeClass<
    RuntimeClassFlags<WinRtClassicComMix>, 
    ABI::Windows::Storage::Streams::IBuffer, 
    Windows::Storage::Streams::IBufferByteAccess, FtmBase>
{
public:
    template<typename T>
    WrapBuffer(T* data, size_t size)
        : _data((void*)data)
        , _size(size)
    {
    }
 
    virtual ~WrapBuffer()
    {
    }
 
    // IBuffer
 
    virtual IFACEMETHODIMP get_Capacity(UINT32* capacity) override
    {
        *capacity = (UINT32)_size;
        return S_OK;
    }
 
    virtual IFACEMETHODIMP get_Length(UINT32* length) override
    {
        *length = (UINT32)_size;
        return S_OK;
    }
 
    virtual IFACEMETHODIMP put_Length(UINT32 length) override
    {
        // This is a WrapBuffer.  We expect to get exactly what we asked for!
        if (length != _size)
            return E_INVALIDARG;
 
        return S_OK;
    }
 
    // IBufferByteAccess
 
    virtual IFACEMETHODIMP Buffer(byte** buffer) override
    {
        *buffer = (byte*)_data;
        return S_OK;
    }
    
private:
    void* _data;
    size_t _size;
};
