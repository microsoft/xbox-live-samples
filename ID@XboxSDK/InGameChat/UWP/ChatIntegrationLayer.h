// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

enum ChatPacketType
{
    IncomingPacket = 0,
    OutgoingPacket = 1
};

namespace Utils
{
    Platform::String^ GetDebugNameForLocalConsole();
}

class ChatIntegrationLayer : public std::enable_shared_from_this<ChatIntegrationLayer>
{
public:
    ChatIntegrationLayer();

    /// <summary>
    /// Initializes the chat manager
    /// </summary>
    void Initialize();

    /// <summary>
    /// Shuts down the chat manager
    /// </summary>
    void Shutdown();

    /// <summary>
    /// Adds a local user to the chat session
    /// </summary>
    /// <param name="xuid">The local user to add</param>
    void AddLocalUser(
        PCWSTR xuid
        );

    /// <summary>
    /// Removes a local user from chat session
    /// </summary>
    /// <param name="xuid">The local user to remove</param>
    void RemoveLocalUser(
        PCWSTR xuid
        );

    /// <summary>
    /// Removes a remote console from chat
    /// </summary>
    /// <param name="uniqueRemoteConsoleIdentifier">A unique ID for the remote console</param>
    void RemoveRemoteConsole(
        uint64_t uniqueRemoteConsoleIdentifier
        );

    /// <summary>
    /// Handles incoming chat messages from the game's network layer
    /// </summary>
    /// <param name="chatPacket">A buffer containing the chat message</param>
    /// <param name="uniqueRemoteConsoleIdentifier">A unique ID for the remote console</param>
    void OnIncomingChatMessage(
        Windows::Storage::Streams::IBuffer^ chatPacket,
        uint64_t uniqueRemoteConsoleIdentifier
        );

    void OnSecureDeviceAssocationConnectionEstablished(
        Microsoft::Xbox::Samples::NetworkMesh::MeshConnection^ connection,
        std::vector<std::wstring> xuids
        );

    /// <summary>
    /// Returns a list of chat users in the chat session
    /// </summary>
    std::vector<std::wstring> GetChatUsersXuids();

    uint8 GetChannelForUser(PCWSTR xuid);

    /// <summary>
    /// Helper function to swap the mute state of a specific chat user
    /// </summary>
    void ChangeChatUserMuteState(
        PCWSTR chatUser
        );

    /// <summary>
    /// Helper function to change the channel of a specific chat user
    /// </summary>
    void ChangeChannelForUser(
        PCWSTR chatUser,
        uint8 newChatChannelIndex
        );

    void ProcessStateChanges();
    void ProcessDataFrames();

    void HandleTextMessage(
        const xbox::services::game_chat_2::game_chat_state_change *change,
        bool transcribed
        );

    xbox::services::game_chat_2::chat_user* GetChatUserByXboxUserId(PCWSTR xuid);

    void RebuildRelationshipMap();

private:
    std::mutex m_lock;
    Windows::Foundation::EventRegistrationToken m_signOutToken;
    bool m_initialized;
    uint64_t m_nextId;
    std::map<std::wstring, uint8> m_channels;
    std::map<uint64_t, Microsoft::Xbox::Samples::NetworkMesh::MeshConnection^> m_consoles;
    std::map<uint64_t, std::vector<std::wstring>> m_consoleUsers;

#if defined(NTDDI_WIN10_RS2) && (NTDDI_VERSION >= NTDDI_WIN10_RS2)
    // UWP Chat overlay is only available in Windows 10 Creators Update (15063) or later
    Windows::Gaming::UI::GameChatOverlay^ m_chatOverlay;
#endif
};

std::shared_ptr<ChatIntegrationLayer> GetChatIntegrationLayer();
