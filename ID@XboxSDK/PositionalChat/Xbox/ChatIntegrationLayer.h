//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved
#pragma once

namespace Utils
{
    Platform::String^ GetDebugNameForLocalConsole();
}

class ChatIntegrationLayer : public std::enable_shared_from_this<ChatIntegrationLayer>
{
public:
    ChatIntegrationLayer();

    void Initialize();
    void Shutdown();

    // Call to add a local console user to gamechat
    void AddLocalUser(
        const wchar_t* xuid
        );

    // Call to remove a local user from gamechat
    void RemoveLocalUser(
        const wchar_t* xuid
        );

    // Remove the users from a remote console from gamechat
    void RemoveRemoteConsole(
        uint64_t uniqueRemoteConsoleIdentifier
        );

    // Handles incoming chat messages from the game's network layer
    void OnIncomingChatMessage(
        Windows::Storage::Streams::IBuffer^ chatPacket,
        uint64_t uniqueRemoteConsoleIdentifier
        );

    // Handles a new SDA connection
    void OnSecureDeviceAssocationConnectionEstablished(
        Microsoft::Xbox::Samples::NetworkMesh::MeshConnection^ connection,
        std::vector<std::wstring> xuids
        );

    // Return a vector of xuids for all gamechat users
    std::vector<std::wstring> GetChatUsersXuids();
    
    // Get the channel a user is in
    uint8 GetChannelForUser(
        const wchar_t* xuid
        );

    // Toggle the mute state of a gamechat user
    void ChangeChatUserMuteState(
        const wchar_t* chatUser
        );

    // Set the channel for a chat user
    void ChangeChannelForUser(
        const wchar_t* chatUser,
        uint8_t newChatChannelIndex
        );

    // Call in update loop to pump chat events
    void ProcessStateChanges();

    // Call to process chat packets
    void ProcessDataFrames();

    // Call to process audio stream state changes
    void ProcessStreamStateChanges();

    // Call to process audio buffers
    void ProcessPostDecodeBuffers();

    // Return the raw gamechat user by xuid
    xbox::services::game_chat_2::chat_user* GetChatUserByXboxUserId(
        const wchar_t* xuid
        );

private:
    void HandleTextMessage(
        const xbox::services::game_chat_2::game_chat_state_change *change,
        bool transcribed
        );

    void RebuildRelationshipMap();

    Concurrency::critical_section m_lock;
    Windows::Foundation::EventRegistrationToken m_signOutToken;
    bool m_initialized;
    uint64_t m_nextId;
    std::map<std::wstring, uint8_t> m_channels;
    std::map<uint64_t, Microsoft::Xbox::Samples::NetworkMesh::MeshConnection^> m_consoles;
    std::map<uint64_t, std::vector<std::wstring>> m_consoleUsers;
};

std::shared_ptr<ChatIntegrationLayer> GetChatIntegrationLayer();
