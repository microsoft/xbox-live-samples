// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <pch.h>
#include "Adv_Identity_Integration.h"

static std::string g_pathPrefix;
std::mutex g_mutex;

#pragma region Advanced Identity Integration Internal
static _Ret_maybenull_ _Post_writable_byte_size_(size) void* __cdecl XalMemAlloc(_In_ size_t size, _In_ uint32_t tag)
{
    UNREFERENCED_PARAMETER(tag);
    return new (std::nothrow) int8_t[size];
}

static void __cdecl XalMemFree(_In_ _Post_invalid_ void* pointer, _In_ uint32_t tag)
{
    UNREFERENCED_PARAMETER(tag);
    delete[] pointer;
}

std::string MakeName(char const* key)
{
    if (!g_pathPrefix.empty())
    {
        return g_pathPrefix + "/" + std::string{ key } + ".json";
    }

    return std::string{ key } + ".json";
}

void OnWrite(
    _In_opt_ void* context,
    _In_opt_ void* userContext,
    _In_ XalPlatformOperation operation,
    _In_z_ char const* key,
    _In_ size_t dataSize,
    _In_reads_bytes_(dataSize) void const* data)
{
    UNREFERENCED_PARAMETER(context);
    UNREFERENCED_PARAMETER(userContext);

    if (dataSize == 0 || data == nullptr)
    {
        return;
    }

    std::string path;

    {
        std::lock_guard<std::mutex> lock(g_mutex);

        path = MakeName(key);

        if (path.empty())
        {
            XalPlatformStorageWriteComplete(operation, XalPlatformOperationResult_Failure);
            return;
        }

        std::ofstream file{ path, std::ios::binary | std::ios::trunc };

        if (!file.is_open())
        {
            XalPlatformStorageWriteComplete(operation, XalPlatformOperationResult_Failure);
            return;
        }

        file.write(reinterpret_cast<char const*>(data), dataSize);

        if (file.bad())
        {
            file.close();
            XalPlatformStorageWriteComplete(operation, XalPlatformOperationResult_Failure);
            return;
        }

        file.close();
    }

    XalPlatformStorageWriteComplete(operation, XalPlatformOperationResult_Success);
}

void OnRead(
    _In_opt_ void* context,
    _In_opt_ void* userContext,
    _In_ XalPlatformOperation operation,
    _In_z_ char const* key)
{
    UNREFERENCED_PARAMETER(context);
    UNREFERENCED_PARAMETER(userContext);

    std::string path;
    std::vector<char> data;

    {
        std::lock_guard<std::mutex> lock(g_mutex);

        path = MakeName(key);

        if (path.empty())
        {
            XalPlatformStorageReadComplete(operation, XalPlatformOperationResult_Failure, 0, nullptr);
            return;
        }

        std::ifstream file{ path, std::ios::binary | std::ios::ate };

        if (!file.is_open())
        {
            // couldn't open the file, let's assume it does not exist
            XalPlatformStorageReadComplete(operation, XalPlatformOperationResult_Success, 0, nullptr);
            return;
        }

        size_t size = file.tellg();
        file.seekg(0, file.beg);

        data.resize(size);

        file.read(data.data(), data.size());

        if (file.bad())
        {
            file.close();
            XalPlatformStorageReadComplete(operation, XalPlatformOperationResult_Failure, 0, nullptr);
            return;
        }

        file.close();
    }

    XalPlatformStorageReadComplete(operation, XalPlatformOperationResult_Success, data.size(), data.data());
}

void OnClear(
    _In_opt_ void* context,
    _In_opt_ void* userContext,
    _In_ XalPlatformOperation operation,
    _In_z_ char const* key)
{
    UNREFERENCED_PARAMETER(context);
    UNREFERENCED_PARAMETER(userContext);

    std::string path;
    std::string path2;

    {
        std::lock_guard<std::mutex> lock(g_mutex);

        path = MakeName(key);

        if (path.empty())
        {
            XalPlatformStorageClearComplete(operation, XalPlatformOperationResult_Failure);
            return;
        }

        int res = std::remove(path.c_str());

        if (res != 0)
        {
            XalPlatformStorageClearComplete(operation, XalPlatformOperationResult_Failure);
            return;
        }
    }

    XalPlatformStorageClearComplete(operation, XalPlatformOperationResult_Success);
}
#pragma endregion //Identity Integration Internal

void Adv_Identity_Init(
    _In_ XTaskQueueHandle asyncQueue,
    _In_opt_ void* context,
    _In_opt_z_ char const* pathPrefix)
{
    // Optional if you want to hook up memory management for XAL
    HRESULT hr = XalMemSetFunctions(&XalMemAlloc, &XalMemFree);
    ASSERT_MESSAGE(SUCCEEDED(hr), "XalMemSetFunctions Failed!");

    // Store PathPrefix to be used later by read/write/clear functions
    g_pathPrefix = pathPrefix;

    XalPlatformStorageEventHandlers storage = {};
    storage.write   = &OnWrite;
    storage.read    = &OnRead;
    storage.clear   = &OnClear;
    storage.context = context;

    hr = XalPlatformStorageSetEventHandlers(asyncQueue, &storage);
    ASSERT_MESSAGE(SUCCEEDED(hr), "XalPlatformStorageSetEventHandlers Failed!");
}