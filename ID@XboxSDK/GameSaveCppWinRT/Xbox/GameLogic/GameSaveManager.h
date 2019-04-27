//--------------------------------------------------------------------------------------
// GameSaveManager.h
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include "GameBoard.h"
#include "GameSave.h"
#include <DirectXMath.h>

#define GAME_BOARD_INDEX_NAME               L"game_board_index"
#define GAME_BOARD_INDEX_DISPLAY_NAME       L"Game Board Index"
#define GAME_BOARD_NAME_PREFIX              L"game_board_"
#define GAME_BOARD_DISPLAY_NAME_PREFIX      L"Game Board "

namespace GameSaveSample
{
    struct GameBoardIndex
    {
        uint32_t m_version = 1;
        uint32_t m_updateCount = 0;
        uint32_t m_activeBoard = 1;
    };

    const std::function<void(GameBoardIndex&)> fnSaveContainerIndex = [](GameBoardIndex& dataToSave)
    {
        dataToSave.m_updateCount++;
    };

    class GameSaveManager
    {
    public:
        // GameSaveManager manages the loading, saving, and PLM handling of the game save blobs that are specific 
        // to this game. These include an index blob and a blob for each game board saved.
        //
        // IMPLEMENTATION NOTE: Here, each blob is saved within its own container. This design enables the sample to 
        // demonstrate the full range of functionality of the Connected Storage API, including sync on demand. 
        // However in a retail game, given the small size of each game save, a better implementation would be to 
        // put all the blobs in one container to reduce the number of calls made to the web service.
        GameSaveManager();

        // Clear the index and games saves loaded in memory and reset the user context
        void Reset();

        // Establish a user context for game saves, syncing with the cloud if possible, and loading the index file (or creating one if needed)
        std::future<HRESULT> InitializeForUser(winrt::Windows::Xbox::System::User const & user, bool refreshOnly);

        // Resume the previously loaded user context
        std::future<HRESULT> ResumeForUser(winrt::Windows::Xbox::System::User const & user);

        // Load or refresh game save container and blob metadata
        // Use containerQuery string to limit update to specific container(s), or leave empty to update all containers
        // Use queryBlobs bool to specify whether or not to query and update blob info - querying blobs has the side effect of forcing a sync for sync-on-demand save contexts, which may be undesirable
        winrt::Windows::Foundation::IAsyncAction LoadContainerMetadata(winrt::hstring const & containerQuery = nullptr, bool queryBlobs = true);

        // Return the current remaining quota in bytes for this user for this title
		std::future<int64_t> GetRemainingQuota();

        // Handle a user signout, saving the index and current game board as needed
        winrt::Windows::Foundation::IAsyncAction OnSignOut();

        // Handle an app suspend, saving the index and current game board as needed
        winrt::Windows::Foundation::IAsyncAction Suspend();

        // Write current container metadata (and optionally blob info) to the game display log
        void WriteGameSaveMetadataToDisplayLog(bool listBlobs);

        //
        // Game Index Tasks
        //

        // Load the index blob that tells us the last board played by the current user
		std::future<bool> LoadIndex();

        // Save the index blob that tells us the last board played by the current user
		std::future<bool> SaveIndex(bool saveOnlyIfDirty);

        //
        // Game Board Tasks
        //

        // Delete the game board container and all its blobs
		std::future<bool> Delete();

        // Delete only the game board blobs, not the container (for developer education only, not relevant to game play)
		std::future<bool> DeleteBlobs();

        // Load the game board from the local game save cache (using GetAsync)
		std::future<bool> Get();

        // Load the game board from the local game save cache (using ReadAsync)
		std::future<bool> Read();

        // Save the game board for uploading to the cloud
		std::future<bool> Save(bool saveOnlyIfDirty);

        // Mark the current game board dirty so that it will be saved automatically when switching to a different board OR during a suspend
        void MarkActiveBoardDirty();

        //
        // GameSaveManager Public Properties
        //

        winrt::Windows::Xbox::Storage::ConnectedStorageSpace GameSaveProvider() { return m_gameSaveProvider; }

        bool HasActiveBoard() { return ActiveBoardNumber() > 0; }

        // Returns true after the first save context has been set (IsSyncOnDemand cannot be changed once a save context has been set until the next time this title is launched)
        static bool HasBeenInitialized() { return s_hasBeenInitialized; }

        // Returns true once the initialization process has completed (regardless of whether the index load was successful)
        bool IsInitialized() { return m_isInitialized; }
        void IsInitialized(bool isInitialized) { m_isInitialized = isInitialized; }

        // Returns true if a sync-on-demand context was specified, or false if a full-sync context was specified (once set, cannot be changed until next app launch)
        bool IsSyncOnDemand() { return m_isSyncOnDemand; }
        void IsSyncOnDemand(bool value)
        {
            if (HasBeenInitialized())
            {
                throw std::runtime_error("cannot change the sync-on-demand setting once a save context has been set");
            }
            m_isSyncOnDemand = value;
        }

        uint32_t IndexUpdateCount()
        {
            if (m_gameBoardIndex != nullptr)
            {
                return m_gameBoardIndex->FrontBuffer().m_updateCount;
            }

            return 0;
        }

        GameBoard& ActiveBoard()
        {
            auto activeBoardGameSave = ActiveBoardGameSave();
            if (activeBoardGameSave == nullptr)
            {
                throw std::runtime_error("trying to retrieve the active GameBoard when none has been set");
            }
            else
            {
                return activeBoardGameSave->FrontBuffer();
            }
        }

        std::shared_ptr<GameSave<GameBoard>> ActiveBoardGameSave()
        {
            auto boardNumber = ActiveBoardNumber();
            auto index = boardNumber - 1;
            if (boardNumber == 0 || index >= m_gameBoardSaves.size())
            {
                return nullptr;
            }
            else
            {
                return m_gameBoardSaves[index];
            }
        }

        // Note: game boards are 1-based; board 0 means there is currently no active board
        uint32_t ActiveBoardNumber()
        {
            if (m_gameBoardIndex != nullptr)
            {
                return m_gameBoardIndex->FrontBuffer().m_activeBoard;
            }

            return 0;
        }

		winrt::Windows::Foundation::IAsyncAction SaveThenLoadContainerAsync(int index)
		{
			// save the current board if it is dirty before switching to another
			bool saveCompleted = co_await Save(true);
			if (saveCompleted)
			{
				co_await LoadContainerMetadata(m_gameBoardSaves[index]->m_containerMetadata->m_containerName);
				{
					GetRemainingQuota();
				}
			}
		}

        void ActiveBoardNumber(uint32_t value)
        {
            if (m_gameBoardIndex != nullptr)
            {
                auto& gameBoardIndex = m_gameBoardIndex->FrontBuffer();
                if (gameBoardIndex.m_activeBoard != value)
                {
                    uint32_t index = gameBoardIndex.m_activeBoard - 1;
                    if (index < m_gameBoardSaves.size())
                    {
						SaveThenLoadContainerAsync(index);
                    }

                    gameBoardIndex.m_activeBoard = value;
                    m_gameBoardIndex->SetData(&gameBoardIndex);
                }
            }
        }

        bool IsActiveBoardDirty()
        {
            auto activeBoardGameSave = ActiveBoardGameSave();
            return activeBoardGameSave != nullptr && activeBoardGameSave->m_isGameDataDirty;
        }

        bool IsSaveIndexDirty()
        {
            return m_gameBoardIndex != nullptr && m_gameBoardIndex->m_isGameDataDirty;
        }

        int64_t RemainingQuotaInBytes()
        {
            return m_remainingQuotaInBytes;
        }

    private:
        void WriteContainerMetadataToDisplayLog(bool listBlobs, std::shared_ptr<GameSaveContainerMetadata> containerMetadata);

        mutable std::mutex                                      m_mutex;
        bool                                                    m_isSuspending;
        bool                                                    m_isSyncOnDemand;
        int64_t                                                 m_remainingQuotaInBytes;

        std::unique_ptr<GameSave<GameBoardIndex>>               m_gameBoardIndex;
        std::vector<std::shared_ptr<GameSave<GameBoard>>>       m_gameBoardSaves;

        winrt::Windows::Xbox::Storage::ConnectedStorageSpace    m_gameSaveProvider;
        winrt::Windows::Xbox::System::User                      m_user;

        static bool                                             s_hasBeenInitialized;
        bool                                                    m_isInitialized;
    };
}
