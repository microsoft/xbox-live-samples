//--------------------------------------------------------------------------------------
// Helpers.h
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include <wrl.h>
#include <winrt\Windows.Storage.Streams.h>

namespace Helpers
{
	struct __declspec(uuid("905a0fef-bc53-11df-8c49-001e4fc686da")) IBufferByteAccess : ::IUnknown
	{
		virtual HRESULT __stdcall Buffer(uint8_t** value) = 0;
	};

    inline byte* GetBufferData(winrt::Windows::Storage::Streams::IBuffer const & buffer)
    {
		winrt::com_ptr<IBufferByteAccess> byte_access = buffer.as<IBufferByteAccess>();
		byte* bytes = nullptr;

		byte_access->Buffer(&bytes);

        return bytes;
    }
}
