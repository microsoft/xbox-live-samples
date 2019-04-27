//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved
#pragma once

#include "ItemRepeater.h"
#include "WICTextureLoader.h"

namespace InGameChat
{
class UserListItem
{
public:
    UserListItem(std::shared_ptr<xbox::services::system::xbox_live_user> user) : _systemUser(user), _chatUser(nullptr), _hasController(true) { }
    UserListItem(xbox::services::game_chat_2::chat_user* user) : _chatUser(user), _systemUser(nullptr), _hasController(true) { }
    UserListItem(std::wstring name) : _name(name), _chatUser(nullptr), _systemUser(nullptr), _hasController(true) { }
    UserListItem(bool hasController = false) : _hasController(hasController), _systemUser(nullptr), _chatUser(nullptr) { }

    enum MicType
    {
        None,
        Mic,
        Kinect
    };

    std::wstring GetName();
    int GetChannel();
    bool IsLocal();
    bool IsMuted();
    bool IsTalking();
    MicType HasMic();

    inline bool HasController() const { return _hasController; }
    inline bool HasUser() const { return !(_systemUser == nullptr && _chatUser == nullptr); }

private:
    std::shared_ptr<xbox::services::system::xbox_live_user> _systemUser;
    xbox::services::game_chat_2::chat_user* _chatUser;
    std::wstring _name;
    bool _hasController;
};

class UserRepeater : public ItemRepeater<std::shared_ptr<UserListItem>>
{
public:
    UserRepeater(
        std::shared_ptr<ATG::UIManager> mgr,
        POINT origin,
        SIZE itemBounds,
        unsigned idBase
    ) :
        ItemRepeater(mgr, origin, itemBounds, idBase),
        _readonly(false)
    {
    }

    void LoadImages(ID3D11Device2* device);
    void SetReadOnly(bool readonly) { _readonly = readonly; }

protected:
    virtual void CreateItem(unsigned index, std::shared_ptr<UserListItem> item, RECT& bounds) override;
    virtual void UpdateItem(unsigned index, std::shared_ptr<UserListItem> item) override;

    inline void LoadImage(ID3D11Device2* device, const wchar_t* name, unsigned int id)
    {
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> tex;

        auto hr = DirectX::CreateWICTextureFromFile(
            device,
            name,
            nullptr,
            tex.GetAddressOf()
        );

        if (SUCCEEDED(hr))
        {
            _mgr->RegisterImage(id, tex.Get());
        }
    }

private:
    bool _readonly;
};
}
