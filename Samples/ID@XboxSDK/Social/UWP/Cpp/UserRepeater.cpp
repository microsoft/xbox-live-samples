// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "pch.h"
#include "UserRepeater.h"

using namespace xbox::services::presence;

void UserRepeater::CreateItem(unsigned index, std::shared_ptr<UserListItem> item, RECT & bounds)
{
    auto base = GetItemId(index);
    auto panel = _mgr->FindPanel<ATG::Overlay>(_panelId);

    // Name
    auto r = bounds;

    r.left += 24;
    r.right = r.left + 400;
    r.top += 5;
    r.bottom = r.top + 40;

    panel->Add(new ATG::TextLabel(base + 1, L"", r, ATG::TextLabel::c_StyleFontSmall));

    // Status
    r = bounds;

    r.left += 500;
    r.right = r.left + 90;
    r.top += 5;
    r.bottom = r.top + 40;

    panel->Add(new ATG::TextLabel(base + 2, L"", r, ATG::TextLabel::c_StyleFontSmall));
}

void UserRepeater::UpdateItem(unsigned index, std::shared_ptr<UserListItem> item)
{
    auto base = GetItemId(index);

    _mgr->FindControl<ATG::TextLabel>(_panelId, base + 1)->SetText(item->GetName().c_str());
    _mgr->FindControl<ATG::TextLabel>(_panelId, base + 2)->SetText(item->GetStatus().c_str());
}

std::wstring UserListItem::GetName()
{
    if (_systemUser != nullptr)
    {
        return std::wstring(_systemUser->display_name());
    }

    return std::wstring();
}

std::wstring UserListItem::GetStatus()
{
    if (_systemUser)
    {
        switch (_systemUser->presence_record().user_state())
        {
            case user_presence_state::away: return L"Away";
            case user_presence_state::offline: return L"Offline";
            case user_presence_state::online: return L"Online";
            case user_presence_state::unknown: return L"Unknown";
            default: return L"Invalid";
        }
    }

    return std::wstring();
}
