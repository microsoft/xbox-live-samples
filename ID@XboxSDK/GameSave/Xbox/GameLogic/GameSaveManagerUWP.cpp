// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "pch.h"
#include "GameSaveManager.h"

using namespace Concurrency;
using namespace Windows::Foundation::Collections;
using namespace Windows::Gaming::XboxLive::Storage;
using namespace xbox::services::system;

namespace GameSaveSample {

bool GameSaveManager::s_hasBeenInitialized = false;

GameSaveManager::GameSaveManager() :
    m_isSuspending(false),
    m_isSyncOnDemand(false)
{
    Reset();
}

void GameSaveManager::Reset()
{
    Log::Write("GameSaveManager::Reset()\n");

    std::lock_guard<std::mutex> lock(m_mutex);
    IsInitialized = false;
    m_isSuspending = false;
    m_remainingQuotaInBytes = 0;
    m_gameBoardIndex.reset();
    m_gameBoardSaves.clear();

    for (auto i = 1; i <= c_saveSlotCount; ++i)
    {
        Platform::String^ containerName = ref new Platform::String(GAME_BOARD_NAME_PREFIX) + i.ToString();
        Platform::String^ containerDisplayName = ref new Platform::String(GAME_BOARD_DISPLAY_NAME_PREFIX) + i.ToString();

        m_gameBoardSaves.push_back(std::make_shared<GameSave<GameBoard>>(containerName, containerDisplayName, fnSaveGameBoard));
    }

    m_gameSaveProvider = nullptr;
    m_user = nullptr;
}

task<HRESULT> GameSaveManager::InitializeForUser(Windows::System::User^ winUser, std::shared_ptr<xbox_live_user> xboxLiveUser, Platform::String^ serviceConfigurationID, bool refreshOnly)
{
    m_serviceConfigurationID = serviceConfigurationID;

    if (winUser == nullptr)
    {
        Log::WriteAndDisplay("ERROR: InitializeForUser called with null winUser\n");
        Reset();

        return create_task([] { return E_INVALIDARG; });
    }

    if (serviceConfigurationID == nullptr)
    {
        Log::WriteAndDisplay("WARNING: InitializeForUser called with null SCID - save context will be offline only\n");
    }

    Log::Write("Windows User NonRoamableId: %ws; SCID: %ws\n", winUser->NonRoamableId->Data(), serviceConfigurationID->Data());
    Log::WriteAndDisplay("InitializeForUser(%ws) (%ws)\n", FormatUserName(xboxLiveUser)->Data(), (IsSyncOnDemand ? L"sync-on-demand" : L"full sync"));

    std::lock_guard<std::mutex> lock(m_mutex);
    m_user = winUser;

    // initiate getting game save manager in sync with the cloud
    Windows::Foundation::IAsyncOperation<GameSaveProviderGetResult^>^ asyncOp;
    if (IsSyncOnDemand)
    {
        asyncOp = Windows::Gaming::XboxLive::Storage::GameSaveProvider::GetSyncOnDemandForUserAsync(winUser, serviceConfigurationID);
    }
    else
    {
        asyncOp = Windows::Gaming::XboxLive::Storage::GameSaveProvider::GetForUserAsync(winUser, serviceConfigurationID);
    }

    auto start = std::chrono::high_resolution_clock::now();

    return create_task(asyncOp).then([this, refreshOnly, start](GameSaveProviderGetResult^ providerResult)
    {
        auto stop = std::chrono::high_resolution_clock::now();
        auto durationMS = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
        if (IsSyncOnDemand)
        {
            Log::WriteAndDisplay("GetSyncOnDemandForUserAsync duration: " + durationMS.ToString() + "ms\n");
        }
        else
        {
            Log::WriteAndDisplay("GetForUserAsync duration: " + durationMS.ToString() + "ms\n");
        }

        m_gameSaveProvider = nullptr;
        Windows::Gaming::XboxLive::Storage::GameSaveProvider^ provider = nullptr;

        if (providerResult->Status == GameSaveErrorStatus::Ok)
        {
            provider = providerResult->Value;
        }
        else
        {
            if (IsSyncOnDemand)
            {
                Log::WriteAndDisplay("ERROR: GetSyncOnDemandForUserAsync result: %ws\n", providerResult->Status.ToString()->Data());
            }
            else
            {
                Log::WriteAndDisplay("ERROR: GetForUserAsync result: %ws\n", providerResult->Status.ToString()->Data());
            }
            return HRESULT(providerResult->Status);
        }

        if (m_user == nullptr)
        {
            Log::WriteAndDisplay("ERROR: GameSaveProvider not set - user became null\n");
            return E_INVALIDARG;
        }

        if (provider != nullptr)
        {
            m_gameSaveProvider = provider;
            s_hasBeenInitialized = true;
            if (refreshOnly)
            {
                Log::WriteAndDisplay("GameSaveProvider refreshed\n");
            }
            else
            {
                Log::WriteAndDisplay("GameSaveProvider created\n");
            }

            if (!m_isSuspending)
            {
                LoadIndex().then([this, refreshOnly](bool)
                {
                    IsInitialized = true;

                    LoadContainerMetadata(nullptr, !IsSyncOnDemand).then([this, refreshOnly](task<void>)
                    {
                        if (!refreshOnly)
                        {
                            WriteGameSaveMetadataToDisplayLog(!IsSyncOnDemand);
                        }

                        GetRemainingQuota();
                    });
                });
            }
        }

        return S_OK;
    });
}

task<HRESULT> GameSaveManager::ResumeForUser(std::shared_ptr<xbox_live_user> xboxLiveUser)
{
    return InitializeForUser(m_user, xboxLiveUser, m_serviceConfigurationID, true);
}

task<void> GameSaveManager::LoadContainerMetadata(Platform::String^ containerQuery, bool queryBlobs)
{
    if (m_gameSaveProvider == nullptr)
    {
        return create_task([] {});
    }

    GameSaveContainerInfoQuery^ query = nullptr;
    try
    {
        query = m_gameSaveProvider->CreateContainerInfoQuery(containerQuery);
    }
    catch (Platform::Exception^ ex)
    {
        Log::WriteAndDisplay("ERROR: CreateContainerInfoQuery threw exception (%ws)\n", GetErrorStringForException(ex)->Data());
        return create_task([] {});
    }

    return create_task(query->GetContainerInfoAsync()).then([this, queryBlobs](GameSaveContainerInfoGetResult^ infoResult) -> task<void>
    {
        if (infoResult->Status != GameSaveErrorStatus::Ok)
        {
            Log::WriteAndDisplay("ERROR: GetContainerInfoAsync result: %ws\n", infoResult->Status.ToString()->Data());
        }
        else
        {
            IVectorView<GameSaveContainerInfo^>^ containerCollection = infoResult->Value;

            if (containerCollection->Size == 0)
            {
                Log::Write("LoadContainerMetadata: no containers found\n");
            }
            else
            {
                std::vector<task<void>> blobQueries;
                auto containerChangeList = m_gameSaveProvider->ContainersChangedSinceLastSync;

                for (auto containerInfo : containerCollection)
                {
                    auto containerName = containerInfo->Name;
                    std::shared_ptr<GameSaveContainerMetadata> gameSaveMetadata;

                    // Match container found in query to an existing GameSave object
                    if (m_gameBoardIndex->m_containerMetadata->m_containerName->Equals(containerName))
                    {
                        gameSaveMetadata = m_gameBoardIndex->m_containerMetadata;
                    }
                    else
                    {
                        auto it = std::find_if(m_gameBoardSaves.begin(), m_gameBoardSaves.end(), [containerName](std::shared_ptr<GameSave<GameBoard>> gameBoardSave)
                        {
                            return gameBoardSave->m_containerMetadata->m_containerName->Equals(containerName);
                        });
                        if (it != m_gameBoardSaves.end())
                        {
                            gameSaveMetadata = (*it)->m_containerMetadata;
                        }
                        else
                        {
                            Log::WriteAndDisplay("WARNING: Found non-game-related container: %ws (%ws) (%llu bytes)\n", containerName->Data(), containerInfo->DisplayName->Data(), containerInfo->TotalSize);
                        }
                    }

                    // Update saved metadata if the query result matches a known type of data
                    if (gameSaveMetadata != nullptr)
                    {
                        gameSaveMetadata->m_isGameDataOnDisk = true;
                        gameSaveMetadata->m_containerDisplayName = containerInfo->DisplayName;
                        gameSaveMetadata->m_lastModified = containerInfo->LastModifiedTime;
                        gameSaveMetadata->m_needsSync = containerInfo->NeedsSync;
                        gameSaveMetadata->m_totalSize = containerInfo->TotalSize;
                        uint32_t changeListIndex = 0;
                        gameSaveMetadata->m_changedSinceLastSync = containerChangeList->IndexOf(containerInfo->Name, &changeListIndex);

                        if (queryBlobs)
                        {
                            // Get blob metadata
                            auto container = m_gameSaveProvider->CreateContainer(containerName);
                            auto blobQuery = container->CreateBlobInfoQuery("");
                            gameSaveMetadata->m_blobs.clear();

                            blobQueries.push_back(create_task(blobQuery->GetBlobInfoAsync()).then([gameSaveMetadata](GameSaveBlobInfoGetResult^ blobInfoResult)
                            {
                                if (blobInfoResult->Status != GameSaveErrorStatus::Ok)
                                {
                                    Log::WriteAndDisplay("ERROR: GetBlobInfoAsync result: %ws for container %ws\n", blobInfoResult->Status.ToString()->Data(), gameSaveMetadata->m_containerName->Data());
                                }
                                else
                                {
                                    IVectorView<GameSaveBlobInfo^>^ blobCollection = blobInfoResult->Value;
                                    for (auto blobInfo : blobCollection)
                                    {
                                        gameSaveMetadata->m_blobs.push_back(GameSaveBlobMetadata(blobInfo->Name, blobInfo->Size));
                                    }
                                }
                            }));
                        }
                    }
                }

                return when_all(std::begin(blobQueries), std::end(blobQueries)).then([]
                {
                    Log::WriteAndDisplay("Game board metadata updated\n");
                });
            }
        }

        return create_task([] {});
    });
}

task<int64_t> GameSaveManager::GetRemainingQuota()
{
    Log::Write("GameSaveManager::GetRemainingQuota()\n");

    if (m_gameSaveProvider == nullptr)
    {
        return create_task([]() -> int64_t { return 0; });
    }

    return create_task(m_gameSaveProvider->GetRemainingBytesInQuotaAsync()).then([this](task<int64_t> t)
    {
        try
        {
            auto remainingQuota = t.get();
            m_remainingQuotaInBytes = remainingQuota;
            Log::WriteAndDisplay("Remaining quota: %lld bytes\n", remainingQuota);
        }
        catch (Platform::Exception^ ex)
        {
            Log::WriteAndDisplay("ERROR: GetRemainingBytesInQuotaAsync task returned exception (%ws)\n", GetErrorStringForException(ex)->Data());
        }

        return m_remainingQuotaInBytes;
    });
}

task<void> GameSaveManager::OnSignOut()
{
    Log::WriteAndDisplay("GameSaveManager::OnSignOut() start...\n");

    return Save(true).then([this](task<bool>)
    {
        return SaveIndex(true);
    }).then([](task<bool>)
    {
        Log::WriteAndDisplay("GameSaveManager::OnSignOut() complete\n");
    });
}

task<void> GameSaveManager::Suspend()
{
    Log::WriteAndDisplay("GameSaveManager::Suspend() start...\n");

    std::lock_guard<std::mutex> lock(m_mutex);
    m_isSuspending = true;

    if (IsInitialized)
    {
        IsInitialized = false;

        return Save(true).then([this](task<bool>)
        {
            return SaveIndex(true);
        }).then([this](task<bool>)
        {
            m_isSuspending = false;
            Log::WriteAndDisplay("GameSaveManager::Suspend() (with save) complete\n");
        });
    }
    else
    {
        m_isSuspending = false;
        return create_task([]
        {
            Log::WriteAndDisplay("GameSaveManager::Suspend() complete\n");
        });
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

task<bool> GameSaveManager::LoadIndex()
{
    if (m_gameSaveProvider == nullptr)
    {
        return create_task([] { return false; });
    }

    Log::WriteAndDisplay("Loading game board index...\n");

    Platform::String^ indexContainerName = ref new Platform::String(GAME_BOARD_INDEX_NAME);
    Platform::String^ indexContainerDisplayName = ref new Platform::String(GAME_BOARD_INDEX_DISPLAY_NAME);
    auto container = m_gameSaveProvider->CreateContainer(indexContainerName);

    m_gameBoardIndex = std::make_unique<GameSave<GameBoardIndex>>(indexContainerName, indexContainerDisplayName, fnSaveContainerIndex);

    return m_gameBoardIndex->Read(container).then([this, container](bool loadSuccess)
    {
        if (loadSuccess)
        {
            Log::WriteAndDisplay("Game board index loaded\n");
            return create_task([] { return true; });
        }
        else
        {
            Log::WriteAndDisplay("Game board index NOT found...creating new one\n");
            return m_gameBoardIndex->Save(container).then([this](bool saveSuccess)
            {
                if (saveSuccess)
                {
                    Log::WriteAndDisplay("Game board index initialization complete\n");
                }
                else
                {
                    Log::WriteAndDisplay("ERROR: Game board index creation FAILED\n");
                }

                return saveSuccess;
            });
        }
    });
}

task<bool> GameSaveManager::SaveIndex(bool saveOnlyIfDirty)
{
    if (m_gameSaveProvider == nullptr)
    {
        return create_task([] { return false; });
    }

    if (saveOnlyIfDirty && !IsSaveIndexDirty)
    {
        return create_task([] { return false; });
    }

    Log::WriteAndDisplay("Saving game board index...\n");

    auto container = m_gameSaveProvider->CreateContainer(m_gameBoardIndex->m_containerMetadata->m_containerName);
    return m_gameBoardIndex->Save(container).then([](bool saveSuccess)
    {
        if (saveSuccess)
        {
            Log::WriteAndDisplay("Game board index saved\n");
        }
        else
        {
            Log::WriteAndDisplay("ERROR: Game board index save FAILED\n");
        }

        return saveSuccess;
    });
}

task<bool> GameSaveManager::Delete()
{
    auto activeBoard = ActiveBoardNumber;
    assert(activeBoard > 0);
    if (m_gameSaveProvider == nullptr)
    {
        return create_task([] { return false; });
    }

    Log::WriteAndDisplay("Deleting game board %d (DeleteContainerAsync)...\n", activeBoard);

    Platform::String^ containerToDelete = m_gameBoardSaves[activeBoard - 1]->m_containerMetadata->m_containerName;

    return create_task(m_gameSaveProvider->DeleteContainerAsync(containerToDelete)).then([=](GameSaveOperationResult^ deleteResult)
    {
        if (deleteResult->Status == GameSaveErrorStatus::Ok)
        {
            Log::WriteAndDisplay("Game board %d deleted\n", activeBoard);
            m_gameBoardSaves[activeBoard - 1]->ResetData();
            return true;
        }
        else
        {
            Log::WriteAndDisplay("ERROR: Game board %d delete FAILED (%ws)\n", activeBoard, deleteResult->Status.ToString()->Data());
        }

        return false;
    });
}

Concurrency::task<bool> GameSaveManager::DeleteBlobs()
{
    auto activeBoard = ActiveBoardNumber;
    assert(activeBoard > 0);
    if (m_gameSaveProvider == nullptr)
    {
        return create_task([] { return false; });
    }

    Log::WriteAndDisplay("Deleting game board %d blobs (SubmitUpdatesAsync)...\n", activeBoard);

    auto container = m_gameSaveProvider->CreateContainer(m_gameBoardSaves[activeBoard - 1]->m_containerMetadata->m_containerName);
    return m_gameBoardSaves[activeBoard - 1]->DeleteBlobs(container).then([activeBoard](bool deleteSuccess)
    {
        if (deleteSuccess)
        {
            Log::WriteAndDisplay("Game board %d blobs deleted\n", activeBoard);
        }
        else
        {
            Log::WriteAndDisplay("ERROR: Game board %d blob delete FAILED\n", activeBoard);
        }

        return deleteSuccess;
    });
}

task<bool> GameSaveManager::Get()
{
    auto activeBoard = ActiveBoardNumber;
    assert(activeBoard > 0);
    if (m_gameSaveProvider == nullptr)
    {
        return create_task([] { return false; });
    }
    
    Log::WriteAndDisplay("Loading game board %d (GetAsync)...\n", activeBoard);

    auto container = m_gameSaveProvider->CreateContainer(m_gameBoardSaves[activeBoard - 1]->m_containerMetadata->m_containerName);
    return m_gameBoardSaves[activeBoard - 1]->Get(container).then([activeBoard](bool loadSuccess)
    {
        if (loadSuccess)
        {
            Log::WriteAndDisplay("Game board %d loaded\n", activeBoard);
        }
        else
        {
            Log::WriteAndDisplay("ERROR: Game board %d load FAILED\n", activeBoard);
        }

        return loadSuccess;
    });
}

task<bool> GameSaveManager::Read()
{
    auto activeBoard = ActiveBoardNumber;
    assert(activeBoard > 0);
    if (m_gameSaveProvider == nullptr)
    {
        return create_task([] { return false; });
    }

    Log::WriteAndDisplay("Loading game board %d (ReadAsync)...\n", activeBoard);

    auto container = m_gameSaveProvider->CreateContainer(m_gameBoardSaves[activeBoard - 1]->m_containerMetadata->m_containerName);
    return m_gameBoardSaves[activeBoard - 1]->Read(container).then([activeBoard](bool loadSuccess)
    {
        if (loadSuccess)
        {
            Log::WriteAndDisplay("Game board %d loaded\n", activeBoard);
        }
        else
        {
            Log::WriteAndDisplay("ERROR: Game board %d load FAILED\n", activeBoard);
        }

        return loadSuccess;
    });
}

task<bool> GameSaveManager::Save(bool saveOnlyIfDirty)
{
    auto activeBoard = ActiveBoardNumber;
    assert(activeBoard > 0);
    if (m_gameSaveProvider == nullptr)
    {
        return create_task([] { return false; });
    }

    if (saveOnlyIfDirty && !m_gameBoardSaves[activeBoard - 1]->m_isGameDataDirty)
    {
        return create_task([] { return false; });
    }

    Log::WriteAndDisplay("Saving game board %d (SubmitUpdatesAsync)...\n", activeBoard);

    auto container = m_gameSaveProvider->CreateContainer(m_gameBoardSaves[activeBoard - 1]->m_containerMetadata->m_containerName);
    return m_gameBoardSaves[activeBoard - 1]->Save(container).then([activeBoard](bool saveSuccess)
    {
        if (saveSuccess)
        {
            Log::WriteAndDisplay("Game board %d saved\n", activeBoard);
        }
        else
        {
            Log::WriteAndDisplay("ERROR: Game board %d save FAILED\n", activeBoard);
        }

        return saveSuccess;
    });
}

void GameSaveManager::MarkActiveBoardDirty()
{
    auto activeGameSave = ActiveBoardGameSave;
    assert(activeGameSave != nullptr);

    if (activeGameSave != nullptr)
    {
        activeGameSave->m_isGameDataDirty = true;
    }
}

void GameSaveManager::WriteContainerMetadataToDisplayLog(bool listBlobs, std::shared_ptr<GameSaveContainerMetadata> containerMetadata)
{
    Platform::String^ containerLog = containerMetadata->m_containerDisplayName;
    containerLog += " (" + containerMetadata->m_totalSize + " bytes)";

    if (containerMetadata->m_needsSync)
    {
        containerLog += " (needs sync)";
    }
    else
    {
        containerLog += " (synced)";
    }

    if (containerMetadata->m_changedSinceLastSync)
    {
        containerLog += " (changed on disk)";
    }

    containerLog += " (" + FormatLocalTimeFromDateTime(containerMetadata->m_lastModified) + ")\n";

    Log::WriteAndDisplay(containerLog);

    if (listBlobs)
    {
        for (const auto& blob : containerMetadata->m_blobs)
        {
            Log::WriteAndDisplay("    %ws (%d bytes)\n", blob.m_blobName->Data(), blob.m_blobSize);
        }
    }
}

} // namespace GameSaveSample
