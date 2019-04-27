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

#if _XDK_VER < 0x295A04AA /* XDK Edition: 160300 */
#error This sample requires the March 2016 XDK or later
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
#include <agile.h>
#include <algorithm>
#include <collection.h>
#include <exception>
#include <functional>
#include <math.h>
#include <memory>
#include <pix.h>
#include <ppltasks.h>
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

// Xbox Live SDK
#ifndef _NOEXCEPT
#define _NOEXCEPT noexcept
#endif

#include <xsapi/services.h>

// Project helpers
#include "DirectXHelper.h"
#include "GlobalConstants.h"
#include "Helpers.h"
#include "Log.h"
#include "StringHelpers.h"

// Define global game accessor
#define Game g_gameInstance
