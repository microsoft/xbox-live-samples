// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

#include "DeviceResources.h"
#include "Texture2D.h"
#include <map>
#include <string>
#include <memory>
#include <set>

namespace GameSaveSample
{
    typedef std::shared_ptr<std::set<std::wstring>> WordList;

    class ContentManager
    {
    public:
        ContentManager(const std::shared_ptr<DX::DeviceResources>& deviceResources);
        ~ContentManager();

        std::shared_ptr<DirectX::Texture2D> LoadTexture(std::wstring path);
        Concurrency::task<WordList> LoadWordList(std::wstring path);

    private:
        // Cached pointer to device resources
        std::shared_ptr<DX::DeviceResources> m_deviceResources;

        std::map<std::wstring, std::shared_ptr<DirectX::Texture2D>> m_textures;
        WordList m_wordList;
    };
}
