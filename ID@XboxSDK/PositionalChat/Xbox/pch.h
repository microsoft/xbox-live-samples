//--------------------------------------------------------------------------------------
// pch.h
//
// Header for standard system include files.
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

// Use the C++ standard templated min/max
#define NOMINMAX

#include <xdk.h>

#if _XDK_VER < 0x42EE0BE0 /* XDK Edition 180500 */
#error This sample requires the May 2018 XDK or later
#endif

#include <wrl.h>
#include <d3d11_x.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <Mmreg.h>

#include <algorithm>
#include <exception>
#include <map>
#include <memory>
#include <stdexcept>
#include <ppltasks.h>
#include <ppl.h>

#include <collection.h>
#include <stdio.h>
#include <pix.h>

#include "Effects.h"
#include "GamePad.h"
#include "GraphicsMemory.h"
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "SimpleMath.h"
#include "CommonStates.h"
#include "PrimitiveBatch.h"
#include "VertexTypes.h"
#include "DDSTextureLoader.h"

#include "GameChat2.h"

namespace DX
{
    // Helper class for COM exceptions
    class com_exception : public std::exception
    {
    public:
        com_exception(HRESULT hr) : result(hr) {}

        virtual const char* what() const override
        {
            static char s_str[64] = {};
            sprintf_s(s_str, "Failure with HRESULT of %08X", result);
            return s_str;
        }

    private:
        HRESULT result;
    };

    // Helper utility converts D3D API failures into exceptions.
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw com_exception(hr);
        }
    }
}