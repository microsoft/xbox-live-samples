//--------------------------------------------------------------------------------------
// pch.h
//
// Header for standard system include files.
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif

#if !defined(NOMINMAX)
#define NOMINMAX
#endif

#if !defined(LIVETK_MULTI_USER_APP)
#define LIVETK_MULTI_USER_APP // for UWP use only, requires package property in appx manifest: <uap:SupportedUsers>multiple</uap:SupportedUsers>
#endif

// Platform-dependent default headers
#ifdef _XBOX_ONE
#include <xdk.h>

#if _XDK_VER < 0x3AD703ED /* XDK Edition: 170300 */
#error This sample requires the March 2017 XDK or later
#endif

#include <d3d11_x.h>
#else
#include <concrt.h>
#include <d3d11_3.h>
#include <dxgi1_6.h>
#include <wincodec.h>
#ifdef _DEBUG
#include <dxgidebug.h>
#endif
#endif

// Standard library and other common API headers
#include <algorithm>
#include <exception>
#pragma warning(push)
#pragma warning(disable : 4702)
#include <functional>
#pragma warning(pop)
#include <future>
#include <math.h>
#include <memory>
#include <pix.h>
#include <stdexcept>
#include <string>
#include <time.h>
#include <vector>
#include <wrl.h>
#include <wrl/implements.h>
#include <wrl/client.h>

// DirectX and DirectXTK classes
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <Audio.h>
#include <CommonStates.h>
#include <GraphicsMemory.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>

// Project helpers
#include "DirectXHelper.h"
#include "GlobalConstants.h"
#include "Helpers.h"
#include "Log.h"
#include "StringHelpers.h"

// Workaround for /permissive- issue specific to the Xbox One XDK winrt/base.h
#define JSCRIPT_E_CANTEXECUTE _HRESULT_TYPEDEF_(0x89020001U)

#include <winrt/Windows.Xbox.Input.h>
#include <winrt/Windows.Xbox.UI.h>
#include <winrt/Windows.Xbox.ApplicationModel.Core.h>
#include <winrt/Windows.Xbox.Storage.h>
#include <winrt/Microsoft.Xbox.Services.h>

#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.ApplicationModel.Activation.h>

#include "LiveResourcesCppWinRT_XDK.h"

// Define global game accessor
#define Game g_gameInstance
