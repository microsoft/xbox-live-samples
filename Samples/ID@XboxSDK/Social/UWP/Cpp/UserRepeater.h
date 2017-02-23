// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#include "ItemRepeater.h"

class UserListItem
{
public:
    UserListItem(xbox::services::social::manager::xbox_social_user* user) : _systemUser(user) { }

    std::wstring GetName();
    std::wstring GetStatus();

private:
    xbox::services::social::manager::xbox_social_user* _systemUser;
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

    void SetReadOnly(bool readonly) { _readonly = readonly; }

protected:
    virtual void CreateItem(unsigned index, std::shared_ptr<UserListItem> item, RECT& bounds) override;
    virtual void UpdateItem(unsigned index, std::shared_ptr<UserListItem> item) override;

private:
    bool _readonly;
};
