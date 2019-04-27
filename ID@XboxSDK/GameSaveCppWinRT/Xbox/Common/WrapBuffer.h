//--------------------------------------------------------------------------------------
// WrapBuffer.h
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include <Windows.Storage.Streams.h>
#include <wrl.h>
#include "Helpers.h"

using Microsoft::WRL::FtmBase;
using Microsoft::WRL::RuntimeClass;
using Microsoft::WRL::RuntimeClassFlags;
using Microsoft::WRL::WinRtClassicComMix;



struct WrapBuffer
	: winrt::implements<WrapBuffer, winrt::Windows::Storage::Streams::IBuffer, Helpers::IBufferByteAccess>

{
public:
    template<typename T>
    WrapBuffer(T* data, size_t size)
        : m_data((void*)data)
        , m_size((int32_t)size)
    {
    }
 
    virtual ~WrapBuffer()
    {
    }
 
    // IBuffer

	virtual int32_t Capacity() const
	{
		return m_size;
	}
 
    virtual IFACEMETHODIMP Capacity(UINT32* capacity)
    {
        *capacity = (UINT32)m_size;
        return S_OK;
    }

 
	virtual int32_t Length() const
	{
		return m_size;
	}

    virtual IFACEMETHODIMP Length(UINT32 length)
    {
        // This is a WrapBuffer.  We expect to get exactly what we asked for!
        if (length != (UINT32)m_size)
            return E_INVALIDARG;
 
        return S_OK;
    }
 
    // IBufferByteAccess
 
    virtual IFACEMETHODIMP Buffer(byte** buffer) override
    {
        *buffer = (byte*)m_data;
        return S_OK;
    }
    
private:
    void* m_data;
    int32_t m_size;
};
