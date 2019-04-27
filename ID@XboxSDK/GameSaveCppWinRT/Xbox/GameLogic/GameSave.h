//--------------------------------------------------------------------------------------
// GameSave.h
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include "Common\Helpers.h"
#include "Common\WrapBuffer.h"
#include "GameSaveContainerMetadata.h"
#include <map>
#include <mutex>
#include <ppltasks.h>
#include <wrl/implements.h>
#include <map>

#include "winrt/Windows.Storage.Streams.h"

#ifdef _DEBUG
#define DEFAULT_MINIMUM_SAVE_SIZE       1024 * 1024 // set this to a value > 0 and <= 16 MB to add padding for the purposes of simulating large game save scenarios, for demos and debugging (DO NOT SHIP A GAME WITH PADDING!)
#else
#define DEFAULT_MINIMUM_SAVE_SIZE       0
#endif

template <typename TData>
class GameSave
{
public:
    GameSave(winrt::hstring const & containerName, winrt::hstring const & containerDisplayName, std::function<void(TData&)> saveFunction, uint32_t minSaveSizeInBytes = static_cast<uint32_t>(DEFAULT_MINIMUM_SAVE_SIZE)) :
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
            Log::Write(L"ERROR: GameSave::ResetData(): memcpy of game save data failed\n");
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
                    Log::Write(L"ERROR: GameSave::SetData(): memcpy of game save data failed\n");
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

	std::future<bool> Get(winrt::Windows::Xbox::Storage::ConnectedStorageContainer const & withContainer)
    {
        Log::Write(L"GameSave::Get(%ws)\n", withContainer.Name().c_str());

		bool getSuccess = co_await GetData(withContainer);
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

            co_return getSuccess;
        }
    }

	std::future<bool> Read(winrt::Windows::Xbox::Storage::ConnectedStorageContainer const & withContainer)
    {
        Log::Write(L"GameSave::Read(%ws)\n", withContainer.Name().c_str());

		bool readSuccess = co_await ReadData(withContainer);
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

            co_return readSuccess;
        }
    }

    std::future<bool> Save(winrt::Windows::Xbox::Storage::ConnectedStorageContainer const & withContainer)
    {
        Log::Write(L"GameSave::Save(%ws)\n", withContainer.Name().c_str());

        bool wasGameDataDirty = m_isGameDataDirty; // preserve the current state of dirtiness in case the save fails
        m_isGameDataDirty = false;

        OnSave(FrontBuffer());

        std::map<winrt::hstring, winrt::Windows::Storage::Streams::IBuffer> updates;
		auto frontBuffer = FrontBuffer();
        auto dataBuffer = MakeDataBuffer(frontBuffer);

        winrt::hstring blobName = L"data";
        updates[L"data"] = dataBuffer;

        auto totalRealBlobSize = dataBuffer.Length();
        if (totalRealBlobSize < m_minSaveSize)
        {
            // a default minimum save size was specified for this game save data (should ONLY be used for debug or demo purposes)
            updates[L"padding"] = MakePaddingBuffer(totalRealBlobSize, true);
        }

		std::vector<winrt::hstring> empty;
		bool saveSuccess = co_await SaveData(withContainer, updates, empty);
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

            co_return saveSuccess;
        }
    }

	std::future<bool> DeleteBlobs(winrt::Windows::Xbox::Storage::ConnectedStorageContainer const & withContainer)
    {
        Log::Write(L"GameSave::DeleteBlobs(%ws)\n", withContainer.Name().c_str());

        std::vector<winrt::hstring> blobsToDelete;
        blobsToDelete.push_back(L"data");
        blobsToDelete.push_back(L"padding");

		std::map<winrt::hstring, winrt::Windows::Storage::Streams::IBuffer> empty;
		bool deleteSuccess = co_await SaveData(withContainer, empty, blobsToDelete);
        {
            if (deleteSuccess)
            {
                ResetData();
            }

            co_return deleteSuccess;
        }
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
    uint32_t                                    m_minSaveSize; // adds padding data so that a save is this minimum size (see DEFAULT_MINIMUM_SAVE_SIZE above, for debugging only)
    mutable std::mutex                          m_mutex;

private:
    winrt::Windows::Foundation::IAsyncOperation<bool> GetData(winrt::Windows::Xbox::Storage::ConnectedStorageContainer const & withContainer)
    {
        using namespace Microsoft::WRL;
        using namespace winrt::Windows::Foundation::Collections;
        using namespace winrt::Windows::Storage::Streams;

        std::vector<winrt::hstring> blobsToRead;
        blobsToRead.push_back(L"data");

		winrt::Windows::Foundation::Collections::IMapView<winrt::hstring, winrt::Windows::Storage::Streams::IBuffer> blobsRead;
		try
		{
			blobsRead = co_await withContainer.GetAsync(std::move(blobsToRead));
		}
		catch (winrt::hresult_error &ex)
		{
			HRESULT code = ex.code();
			if(code == 0x80830008)
			{ 
				Log::Write(L"The operation failed because there is no save data. This is safe to ignore.\n");
			}
			else
			{
				Log::Write(L"The operation failed for another reason\n");
			}
		}
        {
            try
            {
                Log::Write(L"GetAsync task succeeded\n");

                if (blobsRead == nullptr)
                {
                    Log::Write(L"ERROR: GetAsync OK but no blobs in result\n");
                    co_return false;
                }
				
				auto blobBuffer = blobsRead.Lookup(L"data");
                if (blobBuffer == nullptr)
                {
                    Log::Write(L"ERROR: GetAsync OK but data blob not in result\n");
                    co_return false;
                }
				
				byte* blobData = nullptr;
				winrt::com_ptr<Helpers::IBufferByteAccess> byteBufferAccess = blobBuffer.as<Helpers::IBufferByteAccess>();
				byteBufferAccess->Buffer(&blobData);

                co_return SetData(reinterpret_cast<TData*>(blobData), false);
            }
            catch (winrt::hresult_error const & ex)
            {
                auto hr = ex.code();
                if (hr == (int)winrt::Windows::Xbox::Storage::ConnectedStorageErrorStatus::BlobNotFound)
                {
                    Log::WriteAndDisplay(L"GetAsync result: BlobNotFound (%ws)\n", FormatHResult(hr).c_str());
                }
                else
                {
                    Log::WriteAndDisplay(L"GetAsync task returned exception (%ws)\n", FormatHResult(hr).c_str());
                }

                co_return false;
            }
        }
    }

    winrt::Windows::Foundation::IAsyncOperation<bool> ReadData(winrt::Windows::Xbox::Storage::ConnectedStorageContainer const & withContainer)
    {
        std::map<winrt::hstring, winrt::Windows::Storage::Streams::IBuffer> toRead;
		winrt::Windows::Storage::Streams::IBuffer buffer = MakeDataBuffer(BackBuffer());
		toRead.insert(std::pair<winrt::hstring, winrt::Windows::Storage::Streams::IBuffer>(L"data", buffer));

		co_await withContainer.ReadAsync(std::move(toRead));
        {
            bool readSuccess = false;
			
            try
            {
                Log::Write(L"ReadAsync task succeeded\n");

                std::lock_guard<std::mutex> lock(m_mutex);
                SwapBuffers();
                readSuccess = true;
            }
            catch (winrt::hresult_error const & ex)
            {
				auto hr = ex.code();
                if (hr == (int)winrt::Windows::Xbox::Storage::ConnectedStorageErrorStatus::BlobNotFound)
                {
					Log::WriteAndDisplay(L"ReadAsync result: BlobNotFound (%ws)\n", FormatHResult(hr).c_str());
                }
                else
                {
                    Log::WriteAndDisplay(L"ERROR: ReadAsync task returned exception (%ws)\n", FormatHResult(hr).c_str());
                }
            }

            return readSuccess;
        }
    }

    std::future<bool> SaveData
    (
        winrt::Windows::Xbox::Storage::ConnectedStorageContainer const & withContainer,
        std::map<winrt::hstring, winrt::Windows::Storage::Streams::IBuffer> & updates,
        std::vector<winrt::hstring> & deletes
    )
    {
        {
            bool saveSuccess = false;
			
            try
            {
				co_await withContainer.SubmitUpdatesAsync(std::move(updates), std::move(deletes), m_containerMetadata->m_containerDisplayName);
                saveSuccess = true;
                Log::Write(L"SubmitUpdatesAsync task succeeded\n");
            }
            catch (winrt::hresult_error const & ex)
            {
                Log::WriteAndDisplay(L"ERROR: SubmitUpdatesAsync task returned exception (%ws)\n", GetErrorStringForException(ex).c_str());
            }

            co_return saveSuccess;
        }
    }

    winrt::Windows::Storage::Streams::IBuffer MakeDataBuffer(TData& data) const
    {
        using namespace Microsoft::WRL;
        using namespace Microsoft::WRL::Details;
        using namespace winrt::Windows::Storage::Streams;

        Log::Write(L"data buffer = %u bytes\n", sizeof(TData));
		winrt::Windows::Storage::Streams::IBuffer wrapBuffer = winrt::make<WrapBuffer>(&data, sizeof(TData));

		return wrapBuffer;
    }

    winrt::Windows::Storage::Streams::Buffer MakePaddingBuffer(uint32_t size, bool fillWithRandomData) const // (for debugging only)
    {
        using namespace winrt::Windows::Storage::Streams;

        size = m_minSaveSize - size;

		Buffer buffer(size);

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
