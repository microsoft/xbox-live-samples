//--------------------------------------------------------------------------------------
// GameSaveContainerMetadata.h
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include <vector>

struct GameSaveBlobMetadata
{
    GameSaveBlobMetadata(winrt::hstring const & blobName = nullptr, uint32_t blobSize = 0) :
        m_blobName(blobName),
        m_blobSize(blobSize)
    {}

    winrt::hstring              m_blobName;
    uint32_t                    m_blobSize;
};

struct GameSaveContainerMetadata
{
    GameSaveContainerMetadata(winrt::hstring const & containerName, winrt::hstring const & containerDisplayName = nullptr) :
        m_containerDisplayName(containerDisplayName),
        m_containerName(containerName)
    {
        ResetData();
    }

    void ResetData()
    {
        m_blobs.clear();
        m_changedSinceLastSync = false;
        m_isGameDataOnDisk = false;
        m_lastModified = winrt::Windows::Foundation::DateTime::min();
        m_needsSync = false;
        m_totalSize = 0;
    }

    std::vector<GameSaveBlobMetadata>       m_blobs;
    bool                                    m_changedSinceLastSync;
    winrt::hstring                          m_containerDisplayName;
    winrt::hstring                          m_containerName;
    bool                                    m_isGameDataOnDisk;
    winrt::Windows::Foundation::DateTime    m_lastModified;
    bool                                    m_needsSync;
    uint64_t                                m_totalSize;
};
