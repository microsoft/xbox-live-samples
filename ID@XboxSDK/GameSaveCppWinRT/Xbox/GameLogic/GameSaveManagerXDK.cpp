//--------------------------------------------------------------------------------------
// GameSaveManagerXDK.cpp
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "GameSaveManager.h"

using namespace Concurrency;
using namespace winrt::Windows::Xbox::Storage;

namespace GameSaveSample {

bool GameSaveManager::s_hasBeenInitialized = false;

GameSaveManager::GameSaveManager() :
    m_isSuspending(false),
    m_isSyncOnDemand(false),
	m_gameSaveProvider(nullptr),
	m_user(nullptr)
{
    Reset();
}

void GameSaveManager::Reset()
{
    Log::Write(L"GameSaveManager::Reset()\n");

    std::lock_guard<std::mutex> lock(m_mutex);
    IsInitialized(false);
    m_isSuspending = false;
    m_remainingQuotaInBytes = 0;
    m_gameBoardIndex.reset();
    m_gameBoardSaves.clear();

    for (auto i = 1; i <= c_saveSlotCount; ++i)
    {
        wchar_t containerName[100];
        swprintf_s(containerName, GAME_BOARD_NAME_PREFIX L"%i", i);
        wchar_t containerDisplayName[100];
        swprintf_s(containerDisplayName, GAME_BOARD_DISPLAY_NAME_PREFIX L"%i", i);

        m_gameBoardSaves.push_back(std::make_shared<GameSave<GameBoard>>(containerName, containerDisplayName, fnSaveGameBoard));
    }

    m_gameSaveProvider = nullptr;
    m_user = nullptr;
}

std::future<HRESULT> GameSaveManager::InitializeForUser(winrt::Windows::Xbox::System::User const & user, bool refreshOnly)
{
    if (user == nullptr)
    {
        Log::WriteAndDisplay(L"ERROR: InitializeForUser called with null user\n");
        Reset();

		co_return E_INVALIDARG;
    }

    Log::WriteAndDisplay(L"InitializeForUser(%ws) (%ws)\n", FormatUserName(user).c_str(), (IsSyncOnDemand() ? L"sync-on-demand" : L"full sync"));

    m_user = user;

    try
    {
        // initiate getting game save manager in sync with the cloud
        winrt::Windows::Foundation::IAsyncOperation<ConnectedStorageSpace> asyncOp;
        ConnectedStorageSpace provider = nullptr;
        if (IsSyncOnDemand())
        {
            provider = co_await ConnectedStorageSpace::GetSyncOnDemandForUserAsync(std::move(user));
        }
        else
        {
            provider = co_await ConnectedStorageSpace::GetForUserAsync(std::move(user));
        }

        {
            m_gameSaveProvider = nullptr;

            try
            {
                if (m_user == nullptr)
                {
                    Log::WriteAndDisplay(L"ERROR: GameSaveProvider not set - user became null\n");
                    co_return E_INVALIDARG;
                }

                m_gameSaveProvider = provider;
                s_hasBeenInitialized = true;
                if (refreshOnly)
                {
                    Log::WriteAndDisplay(L"GameSaveProvider refreshed\n");
                }
                else
                {
                    Log::WriteAndDisplay(L"GameSaveProvider created\n");
                }

                if (!m_isSuspending)
                {
					co_await LoadIndex();
                    {
                        IsInitialized(true);

						co_await LoadContainerMetadata(L"", !IsSyncOnDemand());
                        {
                            if (!refreshOnly)
                            {
                                WriteGameSaveMetadataToDisplayLog(!IsSyncOnDemand());
                            }

                            GetRemainingQuota();
                        }
                    }
                }
            }
            catch (winrt::hresult_error const & ex)
            {
                Log::WriteAndDisplay(L"ERROR: GetForUserAsync returned exception (%ws)\n", GetErrorStringForException(ex).c_str());
                co_return HRESULT(ex.code());
            }

            co_return S_OK;
        }
    }
    catch (winrt::hresult_error const & ex)
    {
        m_gameSaveProvider = nullptr;
        co_return HRESULT(ex.code());
    }
}

std::future<HRESULT> GameSaveManager::ResumeForUser(winrt::Windows::Xbox::System::User const & user)
{
	std::lock_guard<std::mutex> lock(m_mutex);
    return InitializeForUser(user, true);
}

winrt::Windows::Foundation::IAsyncAction GetAllBlobInfo(BlobInfoQueryResult & blobQuery, std::shared_ptr<GameSaveContainerMetadata> gameSaveMetadata)
{
	auto blobCollection = co_await blobQuery.GetBlobInfoAsync();
	for (auto blobInfo : blobCollection)
	{
		gameSaveMetadata->m_blobs.push_back(GameSaveBlobMetadata(blobInfo.Name, blobInfo.Size));
	}
}

void ReportWhenAllCompleted(std::vector<winrt::Windows::Foundation::IAsyncAction> & actions, const wchar_t* message)
{
	for (auto&& item : actions)
	{
		item.get();
	}

	Log::WriteAndDisplay(message);
}

winrt::Windows::Foundation::IAsyncAction GameSaveManager::LoadContainerMetadata(winrt::hstring const & containerQuery, bool queryBlobs)
{
    if (m_gameSaveProvider == nullptr)
    {
        co_return;
    }

    ContainerInfoQueryResult query = nullptr;
    try
    {
        query = m_gameSaveProvider.CreateContainerInfoQuery(containerQuery);
    }
    catch (winrt::hresult_error const & ex)
    {
        Log::WriteAndDisplay(L"ERROR: CreateContainerInfoQuery threw exception (%ws)\n", GetErrorStringForException(ex).c_str());
		co_return;
    }

	auto containerCollection = co_await query.GetContainerInfo2Async();

    {
        try
        {
            if (containerCollection.Size() == 0)
            {
                Log::Write(L"LoadContainerMetadata: no containers found\n");
            }
            else
            {
                std::vector<winrt::Windows::Foundation::IAsyncAction> blobQueries;

                for (auto containerInfo : containerCollection)
                {
                    auto containerName = containerInfo.Name;
                    std::shared_ptr<GameSaveContainerMetadata> gameSaveMetadata;

                    // Match container found in query to an existing GameSave object
                    if (wcscmp(m_gameBoardIndex->m_containerMetadata->m_containerName.c_str(),containerName.c_str()) == 0)
                    {
                        gameSaveMetadata = m_gameBoardIndex->m_containerMetadata;
                    }
                    else
                    {
                        auto it = std::find_if(m_gameBoardSaves.begin(), m_gameBoardSaves.end(), [containerName](std::shared_ptr<GameSave<GameBoard>> gameBoardSave)
                        {
                            return wcscmp(gameBoardSave->m_containerMetadata->m_containerName.c_str(),containerName.c_str()) == 0;
                        });
                        if (it != m_gameBoardSaves.end())
                        {
                            gameSaveMetadata = (*it)->m_containerMetadata;
                        }
                        else
                        {
                            Log::WriteAndDisplay(L"WARNING: Found non-game-related container: %ws (%ws) (%llu bytes)\n", containerName.c_str(), containerInfo.DisplayName.c_str(), containerInfo.TotalSize);
                        }
                    }

                    // Update saved metadata if the query result matches a known type of data
                    if (gameSaveMetadata != nullptr)
                    {
                        gameSaveMetadata->m_isGameDataOnDisk = true;
                        gameSaveMetadata->m_containerDisplayName = containerInfo.DisplayName;
                        gameSaveMetadata->m_lastModified = containerInfo.LastModifiedTime;
                        gameSaveMetadata->m_needsSync = containerInfo.NeedsSync;
                        gameSaveMetadata->m_totalSize = containerInfo.TotalSize;

                        if (queryBlobs)
                        {
							Log::WriteAndDisplay(L"Querying blob data for %ws\n", gameSaveMetadata->m_containerName.c_str());
                            // Get blob metadata
                            auto container = m_gameSaveProvider.CreateContainer(containerName);
                            auto blobQuery = container.CreateBlobInfoQuery(L"");
                            gameSaveMetadata->m_blobs.clear();

							auto asyncAction = GetAllBlobInfo(blobQuery, gameSaveMetadata);
							blobQueries.push_back(asyncAction);
                        }
                    }
                }

				ReportWhenAllCompleted(blobQueries, L"Game board metadata updated\n");
            }
        }
        catch (winrt::hresult_error const & ex)
        {
            Log::WriteAndDisplay(L"ERROR: GetContainerInfo2Async task returned exception (%ws)\n", GetErrorStringForException(ex).c_str());
        }
    };
}

std::future<int64_t> GameSaveManager::GetRemainingQuota()
{
    Log::Write(L"GameSaveManager::GetRemainingQuota()\n");

    if (m_gameSaveProvider == nullptr)
    {
        co_return 0;
    }

	int64_t quotaResult = co_await m_gameSaveProvider.GetRemainingBytesInQuota64Async();
    {
        try
        {
            m_remainingQuotaInBytes = quotaResult;
            Log::WriteAndDisplay(L"Remaining quota: %lld bytes\n", quotaResult);
        }
        catch (winrt::hresult_error const & ex)
        {
            Log::WriteAndDisplay(L"ERROR: GetRemainingBytesInQuotaAsync task returned exception (%ws)\n", GetErrorStringForException(ex).c_str());
        }

        co_return quotaResult;
    }
}

winrt::Windows::Foundation::IAsyncAction GameSaveManager::OnSignOut()
{
    Log::WriteAndDisplay(L"GameSaveManager::OnSignOut() start...\n");

    co_await Save(true);
    co_await SaveIndex(true);
    Log::WriteAndDisplay(L"GameSaveManager::OnSignOut() complete\n");
}

winrt::Windows::Foundation::IAsyncAction GameSaveManager::Suspend()
{
    Log::WriteAndDisplay(L"GameSaveManager::Suspend() start...\n");

    std::lock_guard<std::mutex> lock(m_mutex);
    m_isSuspending = true;

    if (IsInitialized())
    {
        IsInitialized(false);

        co_await Save(true);
        co_await SaveIndex(true);
        m_isSuspending = false;
        Log::WriteAndDisplay(L"GameSaveManager::Suspend() (with save) complete\n");
    }
    else
    {
        m_isSuspending = false;
        Log::WriteAndDisplay(L"GameSaveManager::Suspend() complete\n");
        co_return;
    }
}

void GameSaveManager::WriteGameSaveMetadataToDisplayLog(bool listBlobs)
{
    if (m_gameBoardIndex == nullptr)
    {
        return;
    }

    WriteContainerMetadataToDisplayLog(listBlobs, m_gameBoardIndex->m_containerMetadata);

    for (auto gameSave : m_gameBoardSaves)
    {
        if (gameSave->m_containerMetadata->m_isGameDataOnDisk)
        {
            WriteContainerMetadataToDisplayLog(listBlobs, gameSave->m_containerMetadata);
        }
    }
}

std::future<bool> GameSaveManager::LoadIndex()
{
    if (m_gameSaveProvider == nullptr)
    {
        co_return false;
    }

    Log::WriteAndDisplay(L"Loading game board index...\n");

    const wchar_t* indexContainerName = GAME_BOARD_INDEX_NAME;
	const wchar_t* indexContainerDisplayName = GAME_BOARD_INDEX_DISPLAY_NAME;
    auto container = m_gameSaveProvider.CreateContainer(indexContainerName);

    m_gameBoardIndex = std::make_unique<GameSave<GameBoardIndex>>(indexContainerName, indexContainerDisplayName, fnSaveContainerIndex);

    auto loadSuccess = co_await m_gameBoardIndex->Read(container);
    {
        if (loadSuccess)
        {
            Log::WriteAndDisplay(L"Game board index loaded\n");
			co_return true;
        }
        else
        {
            Log::WriteAndDisplay(L"Game board index NOT found...creating new one\n");
            bool saveSuccess = co_await m_gameBoardIndex->Save(container);
            {
                if (saveSuccess)
                {
                    Log::WriteAndDisplay(L"Game board index initialization complete\n");
                }
                else
                {
                    Log::WriteAndDisplay(L"ERROR: Game board index creation FAILED\n");
                }

                co_return saveSuccess;
            }
        }
    }
}

std::future<bool> GameSaveManager::SaveIndex(bool saveOnlyIfDirty)
{
    if (m_gameSaveProvider == nullptr)
    {
        co_return false;
    }

    if (saveOnlyIfDirty && !IsSaveIndexDirty())
    {
        co_return false;
    }

    Log::WriteAndDisplay(L"Saving game board index...\n");

    auto container = m_gameSaveProvider.CreateContainer(m_gameBoardIndex->m_containerMetadata->m_containerName);
    bool saveSuccess = co_await m_gameBoardIndex->Save(container);
    {
        if (saveSuccess)
        {
            Log::WriteAndDisplay(L"Game board index saved\n");
        }
        else
        {
            Log::WriteAndDisplay(L"ERROR: Game board index save FAILED\n");
        }

        co_return saveSuccess;
    }
}

std::future<bool> GameSaveManager::Delete()
{
    auto activeBoard = ActiveBoardNumber();
    assert(activeBoard > 0);
    if (m_gameSaveProvider == nullptr)
    {
        co_return false;
    }

    Log::WriteAndDisplay(L"Deleting game board %d (DeleteContainerAsync)...\n", activeBoard);

    const wchar_t* containerToDelete = m_gameBoardSaves[activeBoard - 1]->m_containerMetadata->m_containerName.c_str();

    auto task = m_gameSaveProvider.DeleteContainerAsync(containerToDelete);
    {
        try
        {
            task.get();
            Log::WriteAndDisplay(L"Game board %d deleted\n", activeBoard);
            m_gameBoardSaves[activeBoard - 1]->ResetData();
            co_return true;
        }
        catch (winrt::hresult_error const & ex)
        {
            Log::WriteAndDisplay(L"ERROR: Game board %d delete FAILED (%ws)\n", activeBoard, GetErrorStringForException(ex).c_str());
        }

        co_return false;
    }
}

std::future<bool> GameSaveManager::DeleteBlobs()
{
    auto activeBoard = ActiveBoardNumber();
    assert(activeBoard > 0);
    if (m_gameSaveProvider == nullptr)
    {
        co_return false;
    }

    Log::WriteAndDisplay(L"Deleting game board %d blobs (SubmitUpdatesAsync)...\n", activeBoard);

    auto container = m_gameSaveProvider.CreateContainer(m_gameBoardSaves[activeBoard - 1]->m_containerMetadata->m_containerName);
    bool deleteSuccess = co_await m_gameBoardSaves[activeBoard - 1]->DeleteBlobs(container);
    {
        if (deleteSuccess)
        {
            Log::WriteAndDisplay(L"Game board %d blobs deleted\n", activeBoard);
        }
        else
        {
            Log::WriteAndDisplay(L"ERROR: Game board %d blob delete FAILED\n", activeBoard);
        }

        co_return deleteSuccess;
    }
}

std::future<bool> GameSaveManager::Get()
{
    auto activeBoard = ActiveBoardNumber();
    assert(activeBoard > 0);
    if (m_gameSaveProvider == nullptr)
    {
        co_return false;
    }
    
    Log::WriteAndDisplay(L"Loading game board %d (GetAsync)...\n", activeBoard);

    auto container = m_gameSaveProvider.CreateContainer(m_gameBoardSaves[activeBoard - 1]->m_containerMetadata->m_containerName);
    bool loadSuccess = co_await m_gameBoardSaves[activeBoard - 1]->Get(container);
    {
        if (loadSuccess)
        {
            Log::WriteAndDisplay(L"Game board %d loaded\n", activeBoard);
        }
        else
        {
            Log::WriteAndDisplay(L"ERROR: Game board %d load FAILED\n", activeBoard);
        }

        co_return loadSuccess;
    }
}

std::future<bool> GameSaveManager::Read()
{
    auto activeBoard = ActiveBoardNumber();
    assert(activeBoard > 0);
    if (m_gameSaveProvider == nullptr)
    {
        co_return false;
    }

    Log::WriteAndDisplay(L"Loading game board %d (ReadAsync)...\n", activeBoard);

    auto container = m_gameSaveProvider.CreateContainer(m_gameBoardSaves[activeBoard - 1]->m_containerMetadata->m_containerName);
    bool loadSuccess = co_await m_gameBoardSaves[activeBoard - 1]->Read(container);
    {
        if (loadSuccess)
        {
            Log::WriteAndDisplay(L"Game board %d loaded\n", activeBoard);
        }
        else
        {
            Log::WriteAndDisplay(L"ERROR: Game board %d load FAILED\n", activeBoard);
        }

        co_return loadSuccess;
    }
}

std::future<bool> GameSaveManager::Save(bool saveOnlyIfDirty)
{
    auto activeBoard = ActiveBoardNumber();
    assert(activeBoard > 0);
    if (m_gameSaveProvider == nullptr)
    {
        co_return false;
    }

    if (saveOnlyIfDirty && !m_gameBoardSaves[activeBoard - 1]->m_isGameDataDirty)
    {
        co_return false;
    }

    Log::WriteAndDisplay(L"Saving game board %d (SubmitUpdatesAsync)...\n", activeBoard);

    auto container = m_gameSaveProvider.CreateContainer(m_gameBoardSaves[activeBoard - 1]->m_containerMetadata->m_containerName);
    bool saveSuccess = co_await m_gameBoardSaves[activeBoard - 1]->Save(container);
    {
        if (saveSuccess)
        {
            Log::WriteAndDisplay(L"Game board %d saved\n", activeBoard);
        }
        else
        {
            Log::WriteAndDisplay(L"ERROR: Game board %d save FAILED\n", activeBoard);
        }

        return saveSuccess;
    }
}

void GameSaveManager::MarkActiveBoardDirty()
{
    auto activeGameSave = ActiveBoardGameSave();
    assert(activeGameSave != nullptr);

    if (activeGameSave != nullptr)
    {
        activeGameSave->m_isGameDataDirty = true;
    }
}

void GameSaveManager::WriteContainerMetadataToDisplayLog(bool listBlobs, std::shared_ptr<GameSaveContainerMetadata> containerMetadata)
{
	wchar_t totalSize[30];
	std::wstring containerLog(containerMetadata->m_containerDisplayName.c_str());
	containerLog.append(L" (");

	_i64tow_s(containerMetadata->m_totalSize, totalSize, _countof(totalSize), 10);
	containerLog.append(totalSize);

	containerLog.append(L" bytes)");

    if (containerMetadata->m_needsSync)
    {
        containerLog.append(L" (needs sync)");
    }
    else
    {
		containerLog.append(L" (synced)");
    }

    if (containerMetadata->m_changedSinceLastSync)
    {
        containerLog.append(L" (changed on disk)");
    }

	containerLog.append(L" (");
	containerLog.append(FormatLocalTimeFromDateTime(containerMetadata->m_lastModified).c_str());
	containerLog.append(L")\n");

    Log::WriteAndDisplay(containerLog.c_str());

    if (listBlobs)
    {
        for (const auto& blob : containerMetadata->m_blobs)
        {
            Log::WriteAndDisplay(L"    %ws (%d bytes)\n", blob.m_blobName.c_str(), blob.m_blobSize);
        }
    }
}

} // namespace GameSaveSample
