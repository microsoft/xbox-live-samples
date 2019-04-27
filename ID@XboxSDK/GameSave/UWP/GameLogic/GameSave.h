// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

#include "Common\Helpers.h"
#include "Common\WrapBuffer.h"
#include "GameSaveContainerMetadata.h"
#include <map>
#include <mutex>
#include <ppltasks.h>
#include <wrl/implements.h>

#ifdef _DEBUG
#define DEFAULT_MINIMUM_SAVE_SIZE       1024 * 1024 // set this to a value > 0 and <= 16 MB to add padding for the purposes of simulating large game save scenarios, for demos and debugging (DO NOT SHIP A GAME WITH PADDING!)
#else
#define DEFAULT_MINIMUM_SAVE_SIZE       0
#endif

template <typename TData>
class GameSave
{
public:
    GameSave(Platform::String^ containerName, Platform::String^ containerDisplayName, std::function<void(TData&)> saveFunction, uint32 minSaveSizeInBytes = static_cast<uint32>(DEFAULT_MINIMUM_SAVE_SIZE)) :
        OnSave(saveFunction),
        m_isGameDataDirty(false),
        m_isGameDataLoaded(false),
        m_minSaveSize(minSaveSizeInBytes),
        m_currentDataBuffer(0)
    {
        m_containerMetadata = std::make_shared<GameSaveContainerMetadata>(containerName, containerDisplayName);
    }

    inline bool operator==(const GameSave& rhs)
    {
        return m_containerName->Equals(rhs.m_containerName);
    }

    inline bool operator!=(const GameSave& rhs)
    {
        return !(*this == rhs);
    }

    std::function<void (TData&)> OnSave; // always run this function on the data in the front buffer during a save

    void ResetData()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_isGameDataDirty = false;
        m_isGameDataLoaded = false;
        m_containerMetadata->ResetData();

        TData emptyData;
        auto err = memcpy_s(&BackBuffer(), sizeof(TData), &emptyData, sizeof(TData));
        if (err != 0)
        {
            Log::Write("ERROR: GameSave::ResetData(): memcpy of game save data failed\n");
            return;
        }

        SwapBuffers();
    }

    bool SetData(TData* newData, bool markDirtyOnSuccess = true)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (newData != nullptr && newData != &FrontBuffer())
        {
            TData* backBuffer = &BackBuffer();
            if (newData != backBuffer)
            {
                auto err = memcpy_s(backBuffer, sizeof(TData), newData, sizeof(TData));
                if (err != 0)
                {
                    Log::Write("ERROR: GameSave::SetData(): memcpy of game save data failed\n");
                    return false;
                }
            }

            SwapBuffers();
        }

        if (markDirtyOnSuccess)
        {
            m_isGameDataDirty = true;
        }

        return true;
    }

#ifdef _XBOX_ONE
    Concurrency::task<bool> Get(Windows::Xbox::Storage::ConnectedStorageContainer^ withContainer)
#else
    Concurrency::task<bool> Get(Windows::Gaming::XboxLive::Storage::GameSaveContainer^ withContainer)
#endif
    {
        Log::Write("GameSave::Get(%ws)\n", withContainer->Name->Data());

        return GetData(withContainer).then([this](bool getSuccess)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_isGameDataLoaded)
            {
                m_isGameDataLoaded = getSuccess;
            }

            if (getSuccess)
            {
                m_isGameDataDirty = false;
            }

            return getSuccess;
        });
    }

#ifdef _XBOX_ONE
    Concurrency::task<bool> Read(Windows::Xbox::Storage::ConnectedStorageContainer^ withContainer)
#else
    Concurrency::task<bool> Read(Windows::Gaming::XboxLive::Storage::GameSaveContainer^ withContainer)
#endif
    {
        Log::Write("GameSave::Read(%ws)\n", withContainer->Name->Data());

        return ReadData(withContainer).then([this](bool readSuccess)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_isGameDataLoaded)
            {
                m_isGameDataLoaded = readSuccess;
            }

            if (readSuccess)
            {
                m_isGameDataDirty = false;
            }

            return readSuccess;
        });
    }

#ifdef _XBOX_ONE
    Concurrency::task<bool> Save(Windows::Xbox::Storage::ConnectedStorageContainer^ withContainer)
#else
    Concurrency::task<bool> Save(Windows::Gaming::XboxLive::Storage::GameSaveContainer^ withContainer)
#endif
    {
        Log::Write("GameSave::Save(%ws)\n", withContainer->Name->Data());

        bool wasGameDataDirty = m_isGameDataDirty; // preserve the current state of dirtiness in case the save fails
        m_isGameDataDirty = false;

        OnSave(FrontBuffer());

        auto updates = ref new Platform::Collections::Map<Platform::String^, Windows::Storage::Streams::IBuffer^>();
        auto dataBuffer = MakeDataBuffer(FrontBuffer());

        Platform::String^ blobName = "data";
        updates->Insert(blobName, dataBuffer);

        auto totalRealBlobSize = dataBuffer->Length;
        if (totalRealBlobSize < m_minSaveSize)
        {
            // a default minimum save size was specified for this game save data (should ONLY be used for debug or demo purposes)
            updates->Insert("padding", MakePaddingBuffer(totalRealBlobSize, true));
        }

        return SaveData(withContainer, updates->GetView(), nullptr).then([this, wasGameDataDirty](bool saveSuccess)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_isGameDataLoaded)
            {
                m_isGameDataLoaded = saveSuccess;
            }

            if (wasGameDataDirty && !saveSuccess)
            {
                m_isGameDataDirty = true;
            }

            return saveSuccess;
        });
    }

#ifdef _XBOX_ONE
    Concurrency::task<bool> DeleteBlobs(Windows::Xbox::Storage::ConnectedStorageContainer^ withContainer)
#else
    Concurrency::task<bool> DeleteBlobs(Windows::Gaming::XboxLive::Storage::GameSaveContainer^ withContainer)
#endif
    {
        Log::Write("GameSave::DeleteBlobs(%ws)\n", withContainer->Name->Data());

        std::vector<Platform::String^> blobsToDelete;
        blobsToDelete.push_back(L"data");
        blobsToDelete.push_back(L"padding");

        return SaveData(withContainer, nullptr, ref new Platform::Collections::VectorView<Platform::String^>(blobsToDelete)).then([this](bool deleteSuccess)
        {
            if (deleteSuccess)
            {
                ResetData();
            }

            return deleteSuccess;
        });
    }

    const TData& FrontBuffer() const
    {
        return m_data[m_currentDataBuffer];
    }

    TData& FrontBuffer()
    {
        return m_data[m_currentDataBuffer];
    }


    std::shared_ptr<GameSaveContainerMetadata>  m_containerMetadata;
    bool                                        m_isGameDataDirty;
    bool                                        m_isGameDataLoaded;
    uint32                                      m_minSaveSize; // adds padding data so that a save is this minimum size (see DEFAULT_MINIMUM_SAVE_SIZE above, for debugging only)
    mutable std::mutex                          m_mutex;

private:
#ifdef _XBOX_ONE
    Concurrency::task<bool> GetData(Windows::Xbox::Storage::ConnectedStorageContainer^ withContainer)
    {
        using namespace Microsoft::WRL;
        using namespace Platform::Collections;
        using namespace Windows::Foundation::Collections;
        using namespace Windows::Storage::Streams;

        std::vector<Platform::String^> blobsToRead;
        blobsToRead.push_back("data");

        auto start = std::chrono::high_resolution_clock::now();

        return create_task(withContainer->GetAsync(ref new VectorView<Platform::String^>(blobsToRead))).then([this, start](task<IMapView<Platform::String^, IBuffer^>^> t)
        {
            auto stop = std::chrono::high_resolution_clock::now();
            auto durationMS = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
            Log::WriteAndDisplay("GetAsync duration: " + durationMS.ToString() + "ms\n");

            try
            {
                auto blobsRead = t.get();
                Log::Write(L"GetAsync task succeeded\n");

                if (blobsRead == nullptr)
                {
                    Log::Write("ERROR: GetAsync OK but no blobs in result\n");
                    return false;
                }

                auto blobBuffer = blobsRead->Lookup("data");
                if (blobBuffer == nullptr)
                {
                    Log::Write("ERROR: GetAsync OK but data blob not in result\n");
                    return false;
                }

                IUnknown* unknown = reinterpret_cast<IUnknown*>(blobBuffer);
                ComPtr<IBufferByteAccess> bufferByteAccess;
                HRESULT hr = unknown->QueryInterface(_uuidof(IBufferByteAccess), &bufferByteAccess);
                if (FAILED(hr))
                {
                    Log::Write("ERROR: GetAsync OK but QueryInterface failed to get IBufferByteAccess\n");
                    return false;
                }

                byte* blobData = nullptr;
                bufferByteAccess->Buffer(&blobData);

                return SetData(reinterpret_cast<TData*>(blobData), false);
            }
            catch (Platform::Exception^ ex)
            {
                auto hr = ex->HResult;
                if (hr == (int)Windows::Xbox::Storage::ConnectedStorageErrorStatus::BlobNotFound)
                {
                    Log::WriteAndDisplay("GetAsync result: BlobNotFound (%ws)\n", FormatHResult(hr)->Data());
                }
                else
                {
                    Log::WriteAndDisplay("GetAsync task returned exception (%ws)\n", FormatHResult(hr)->Data());
                }

                return false;
            }
        });
    }
#else
    Concurrency::task<bool> GetData(Windows::Gaming::XboxLive::Storage::GameSaveContainer^ withContainer)
    {
        using namespace Microsoft::WRL;
        using namespace Platform::Collections;
        using namespace Windows::Foundation::Collections;
        using namespace Windows::Gaming::XboxLive::Storage;
        using namespace Windows::Storage::Streams;

        std::vector<Platform::String^> blobsToRead;
        blobsToRead.push_back("data");

        auto start = std::chrono::high_resolution_clock::now();

        return create_task(withContainer->GetAsync(ref new VectorView<Platform::String^>(blobsToRead))).then([this, start](GameSaveBlobGetResult^ getResult)
        {
            auto stop = std::chrono::high_resolution_clock::now();
            auto durationMS = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
            Log::WriteAndDisplay("GetAsync duration: " + durationMS.ToString() + "ms\n");

            switch (getResult->Status)
            {
            case GameSaveErrorStatus::BlobNotFound:
                Log::WriteAndDisplay("GetAsync result: BlobNotFound\n");
                return false;
            case GameSaveErrorStatus::Ok:
            {
                IMapView<Platform::String^, IBuffer^>^ blobsRead = getResult->Value;
                if (blobsRead == nullptr)
                {
                    Log::Write("ERROR: GetAsync OK but no blobs in result\n");
                    return false;
                }

                auto blobBuffer = blobsRead->Lookup("data");
                if (blobBuffer == nullptr)
                {
                    Log::Write("ERROR: GetAsync OK but data blob not in result\n");
                    return false;
                }

                IUnknown* unknown = reinterpret_cast<IUnknown*>(blobBuffer);
                ComPtr<IBufferByteAccess> bufferByteAccess;
                HRESULT hr = unknown->QueryInterface(_uuidof(IBufferByteAccess), &bufferByteAccess);
                if (FAILED(hr))
                {
                    Log::Write("ERROR: GetAsync OK but QueryInterface failed to get IBufferByteAccess\n");
                    return false;
                }

                byte* blobData = nullptr;
                bufferByteAccess->Buffer(&blobData);

                return SetData(reinterpret_cast<TData*>(blobData), false);
            }
            default:
                Log::WriteAndDisplay("GetAsync result: %ws\n", getResult->Status.ToString()->Data());
                return false;
            }
        });
    }
#endif

#ifdef _XBOX_ONE
    Concurrency::task<bool> ReadData(Windows::Xbox::Storage::ConnectedStorageContainer^ withContainer)
    {
        using namespace Platform::Collections;
        using namespace Windows::Storage::Streams;

        std::map<Platform::String^, IBuffer^> toRead;
        toRead["data"] = MakeDataBuffer(BackBuffer());

        auto start = std::chrono::high_resolution_clock::now();

        return create_task(withContainer->ReadAsync(ref new MapView<Platform::String^, IBuffer^>(toRead))).then([=](task<void> t)
        {
            auto stop = std::chrono::high_resolution_clock::now();
            auto durationMS = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
            Log::WriteAndDisplay("ReadAsync duration: " + durationMS.ToString() + "ms\n");

            bool readSuccess = false;

            try
            {
                t.get();
                Log::Write("ReadAsync task succeeded\n");

                std::lock_guard<std::mutex> lock(m_mutex);
                SwapBuffers();
                readSuccess = true;
            }
            catch (Platform::Exception^ ex)
            {
                auto hr = ex->HResult;
                if (hr == (int)Windows::Xbox::Storage::ConnectedStorageErrorStatus::BlobNotFound)
                {
                    Log::WriteAndDisplay("ReadAsync result: BlobNotFound (%ws)\n", FormatHResult(hr)->Data());
                }
                else
                {
                    Log::WriteAndDisplay("ERROR: ReadAsync task returned exception (%ws)\n", FormatHResult(hr)->Data());
                }
            }

            return readSuccess;
        });
    }
#else
    Concurrency::task<bool> ReadData(Windows::Gaming::XboxLive::Storage::GameSaveContainer^ withContainer)
    {
        using namespace Platform::Collections;
        using namespace Windows::Gaming::XboxLive::Storage;
        using namespace Windows::Storage::Streams;

        std::map<Platform::String^, IBuffer^> toRead;
        toRead["data"] = MakeDataBuffer(BackBuffer());

        auto start = std::chrono::high_resolution_clock::now();

        return create_task(withContainer->ReadAsync(ref new MapView<Platform::String^, IBuffer^>(toRead))).then([=](GameSaveOperationResult^ readResult)
        {
            auto stop = std::chrono::high_resolution_clock::now();
            auto durationMS = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
            Log::WriteAndDisplay("ReadAsync duration: " + durationMS.ToString() + "ms\n");

            bool readSuccess = false;

            switch (readResult->Status)
            {
            case GameSaveErrorStatus::BlobNotFound:
                Log::WriteAndDisplay("ReadAsync result: BlobNotFound\n");
                break;
            case GameSaveErrorStatus::Ok:
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                SwapBuffers();
                readSuccess = true;
                break;
            }
            default:
                Log::WriteAndDisplay("ReadAsync result: %ws\n", readResult->Status.ToString()->Data());
            }

            return readSuccess;
        });
    }
#endif

#ifdef _XBOX_ONE
    Concurrency::task<bool> SaveData
    (
        Windows::Xbox::Storage::ConnectedStorageContainer^ withContainer,
        Windows::Foundation::Collections::IMapView<Platform::String^, Windows::Storage::Streams::IBuffer^>^ updates,
        Platform::Collections::VectorView<Platform::String^>^ deletes
    )
    {
        auto start = std::chrono::high_resolution_clock::now();

        auto submitUpdatesOp = withContainer->SubmitUpdatesAsync(updates, deletes, m_containerMetadata->m_containerDisplayName);
        return create_task(submitUpdatesOp).then([start](task<void> t)
        {
            auto stop = std::chrono::high_resolution_clock::now();
            auto durationMS = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
            Log::WriteAndDisplay("SubmitUpdatesAsync duration: " + durationMS.ToString() + "ms\n");

            bool saveSuccess = false;

            try
            {
                t.get();
                saveSuccess = true;
                Log::Write("SubmitUpdatesAsync task succeeded\n");
            }
            catch (Platform::Exception^ ex)
            {
                Log::WriteAndDisplay("ERROR: SubmitUpdatesAsync task returned exception (%ws)\n", GetErrorStringForException(ex)->Data());
            }

            return saveSuccess;
        });
    }
#else
    Concurrency::task<bool> SaveData
    (
        Windows::Gaming::XboxLive::Storage::GameSaveContainer^ withContainer,
        Windows::Foundation::Collections::IMapView<Platform::String^, Windows::Storage::Streams::IBuffer^>^ updates,
        Platform::Collections::VectorView<Platform::String^>^ deletes
    )
    {
        using namespace Windows::Gaming::XboxLive::Storage;

        auto start = std::chrono::high_resolution_clock::now();

        auto submitUpdatesOp = withContainer->SubmitUpdatesAsync(updates, deletes, m_containerMetadata->m_containerDisplayName);
        return create_task(submitUpdatesOp).then([start](GameSaveOperationResult^ submitResult)
        {
            auto stop = std::chrono::high_resolution_clock::now();
            auto durationMS = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
            Log::WriteAndDisplay("SubmitUpdatesAsync duration: " + durationMS.ToString() + "ms\n");

            bool saveSuccess = false;

            if (submitResult->Status == GameSaveErrorStatus::Ok)
            {
                saveSuccess = true;
                Log::Write("SubmitUpdatesAsync task succeeded\n");
            }
            else
            {
                Log::WriteAndDisplay("ERROR: SubmitUpdatesAsync task returned error result (%ws (%ws))\n", 
                    submitResult->Status.ToString()->Data(), 
                    FormatHResult(static_cast<int>(submitResult->Status))->Data());
            }

            return saveSuccess;
        });
    }
#endif

    Windows::Storage::Streams::IBuffer^ MakeDataBuffer(TData& data) const
    {
        using namespace Microsoft::WRL;
        using namespace Microsoft::WRL::Details;
        using namespace Windows::Storage::Streams;

        Log::Write("data buffer = %u bytes\n", sizeof(TData));
        ComPtr<ABI::Windows::Storage::Streams::IBuffer> wrapBuffer = Make<WrapBuffer>(&data, sizeof(TData));

        return reinterpret_cast<IBuffer^>(wrapBuffer.Get());
    }

    Windows::Storage::Streams::Buffer^ MakePaddingBuffer(uint32 size, bool fillWithRandomData) const // (for debugging only)
    {
        using namespace Windows::Storage::Streams;

        size = m_minSaveSize - size;

        Buffer^ buffer = ref new Buffer(size);
        buffer->Length = size;

        if (fillWithRandomData)
        {
            int* data = reinterpret_cast<int*>(Helpers::GetBufferData(buffer));
            size_t randNumbersToWrite = size / sizeof(*data);
            for (size_t i = 0; i < randNumbersToWrite; ++i)
            {
                data[i] = rand();
            }
        }

        return buffer;
    }

    TData& BackBuffer()
    {
        return m_data[(m_currentDataBuffer + 1) & 1];
    }

    void SwapBuffers()
    {
        m_currentDataBuffer = (m_currentDataBuffer + 1) & 1;
    }

    size_t                  m_currentDataBuffer;
    TData                   m_data[2];
};
