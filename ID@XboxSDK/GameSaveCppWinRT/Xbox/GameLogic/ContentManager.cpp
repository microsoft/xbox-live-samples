//--------------------------------------------------------------------------------------
// ContentManager.cpp
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "ContentManager.h"

#include <algorithm>
#include <cctype>
#include <fstream>

#include <codecvt>

using namespace Concurrency;
using namespace DirectX;

namespace GameSaveSample {

ContentManager::ContentManager(const std::shared_ptr<DX::DeviceResources>& deviceResources)
    : m_deviceResources(deviceResources)
{
}

ContentManager::~ContentManager()
{
    m_textures.clear();
}

std::shared_ptr<Texture2D> ContentManager::LoadTexture(std::wstring path)
{
    // Lower case the path for our map
    std::transform(path.begin(), path.end(), path.begin(), towlower);

    // Look in our cache first
    auto itr = m_textures.find(path);
    if (itr != m_textures.end())
    {
        return itr->second;
    }

    // Otherwise use the WICTextureLoader APIs to load the texture into our cache
    std::shared_ptr<Texture2D> texture(Texture2D::FromFile(m_deviceResources->GetD3DDevice(), path));
    m_textures[path] = texture;

    return texture;
}

std::future<WordList> ContentManager::LoadWordList(std::wstring path)
{
    // Look in our cache first
    if (m_wordList != nullptr && !m_wordList->empty())
    {
        //return create_task([this] { return m_wordList; });
		co_return m_wordList;
    }

    //return create_task([this, path]
    {
        // Otherwise load the word list into our cache
        m_wordList.reset(new std::set<std::wstring>);
        std::wstring lowercasePath = path;
        std::transform(path.begin(), path.end(), lowercasePath.begin(), towlower);

        HANDLE hWordFile = CreateFile2(lowercasePath.c_str(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, nullptr);
        {
            if (hWordFile == INVALID_HANDLE_VALUE)
            {
                Log::Write(L"ERROR opening word list file (%ws)\n", FormatHResult(E_INVALIDARG).c_str());
                return m_wordList;
            }

            FILE_STANDARD_INFO fileInfo;
            if (GetFileInformationByHandleEx(hWordFile, FileStandardInfo, &fileInfo, sizeof(fileInfo)) == FALSE)
            {
                Log::Write(L"ERROR getting file size of word list file (%ws)\n", FormatHResult(HRESULT_FROM_WIN32(GetLastError())).c_str());
                goto ExitFn;
            }

            LARGE_INTEGER fileSize = fileInfo.EndOfFile;
            if (fileSize.HighPart > 0)
            {
                Log::Write(L"ERROR file size of word list too big\n");
                goto ExitFn;
            }

            std::unique_ptr<char[]> buffer;
            buffer.reset(new char[fileSize.LowPart + 1]);

            DWORD bytesRead;
            if (ReadFile(hWordFile, buffer.get(), fileSize.LowPart, &bytesRead, nullptr) == FALSE)
            {
                Log::Write(L"ERROR reading word list file (%ws)\n", FormatHResult(HRESULT_FROM_WIN32(GetLastError())).c_str());
                goto ExitFn;
            }

            if (bytesRead < fileSize.LowPart)
            {
                Log::Write(L"ERROR incomplete read of word list file (%d of %d bytes read)\n", bytesRead, fileSize.LowPart);
                goto ExitFn;
            }

            auto readBuffer = buffer.get();
            readBuffer[bytesRead] = 0;
            char* word = 0;
            char* token = 0;
            size_t converted = 0;
            wchar_t wword[11] = {}; // longest word supported
            word = strtok_s(readBuffer, "\r\n", &token);
            while (word != nullptr)
            {
                mbstowcs_s(&converted, wword, word, 10);
                m_wordList->insert(wword);
                word = strtok_s(nullptr, "\r\n", &token);
            }
        }

    ExitFn:
        if (hWordFile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hWordFile);
        }

        co_return m_wordList;
    }
}
} // namespace GameSaveSample
