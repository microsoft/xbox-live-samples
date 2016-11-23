//--------------------------------------------------------------------------------------
// File: SessionListView.h
//
// A helper class for managing and displaying a list of data
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-------------------------------------------------------------------------------------
#pragma once

// Helper UI class for displaying Multiplayer Session View
class SessionViewRow
{
public:
    ATG::TextLabel *m_xuid;
    ATG::TextLabel *m_skill;
    ATG::TextLabel *m_health;
    ATG::Image *m_gamerpic;
    ATG::Image *m_hostIcon;
};

// Helper UI class for displaying Multiplayer Session View
class SessionView
{
public:
    SessionView() { m_rows.resize(4); }
    void Show();
    void Hide();
    void Clear();
    void SetVisibility(bool visible);
    void SetControls(ATG::IPanel *parent, int rowStart);

    ATG::TextLabel* m_sessionNameLabel;
    ATG::TextLabel* m_gameModeLabel;
    ATG::TextLabel* m_mapLabel;
    std::vector<SessionViewRow> m_rows;
};
