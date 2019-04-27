//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include <mmreg.h>
#include "Robuffer.h"
#include "WrapBuffer.h"
#include "ChatIntegrationLayer.h"
#include "InGameChat.h"
#include "GameChat2Impl.h"

using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;
using namespace Windows::Xbox::System;
using namespace Windows::Foundation;
using namespace Microsoft::WRL::Details;
using namespace Windows::Storage::Streams;
using namespace xbox::services::game_chat_2;

using Microsoft::WRL::ComPtr;

namespace Utils
{
    void GetBufferBytes(Windows::Storage::Streams::IBuffer^ inbuf, byte **outbuf)
    {
        ComPtr<IInspectable> tempb(reinterpret_cast<IInspectable*>(inbuf));
        ComPtr<IBufferByteAccess> bufferaccess;

        tempb.As(&bufferaccess);
        bufferaccess->Buffer(outbuf);
    }

    Platform::String^ GetDebugNameForLocalConsole()
    {
        // For debug name, see if there is a local signed-in user
        Platform::String^ debugName = L"";
        try
        {
            for (User^ user : User::Users)
            {
                if (user->IsSignedIn)
                {
                    debugName = user->DisplayInfo->Gamertag + L"'s console";
                    break;
                }
            }
        }
        catch (...)
        {
            // just return an empty string
        }

        return debugName;
    }
}

std::shared_ptr<ChatIntegrationLayer> GetChatIntegrationLayer()
{
    static std::shared_ptr<ChatIntegrationLayer> chatIntegrationLayerInstance;

    if (chatIntegrationLayerInstance == nullptr)
    {
        chatIntegrationLayerInstance.reset(new ChatIntegrationLayer());
    }

    return chatIntegrationLayerInstance;
}

ChatIntegrationLayer::ChatIntegrationLayer()
{
    m_nextId = 1;
    m_initialized = false;
}

void ChatIntegrationLayer::Initialize()
{
    DebugTrace("ChatIntegrationLayer::Initialize()\n");

    auto &chatManager = chat_manager::singleton_instance();

    Windows::Xbox::UI::Accessibility::SetSpeechToTextPositionHint(
        Windows::Xbox::UI::SpeechToTextPositionHint::MiddleRight
        );

    // Put the audio processing thread on core 5
    chat_manager::set_thread_processor(
        game_chat_thread_id::audio,
        4
        );

    // Put the network processing thread on core 6
    chat_manager::set_thread_processor(
        game_chat_thread_id::networking,
        5
        );

    // Initialize chat manager
    chatManager.initialize(
        Sample::MAXUSERS,                                                                // Max users
        1.0f,                                                                            // Default volume
        xbox::services::game_chat_2::c_communicationRelationshipSendAndReceiveAll,       // Default to 'everyone can talk'
        game_chat_shared_device_communication_relationship_resolution_mode::restrictive, // Be restrictive for shared device (kinect)
        game_chat_speech_to_text_conversion_mode::automatic                              // Let GameChat perform perform accessibility actions
        );

    // Set the default bitrate to 'medium/normal' quality
    chatManager.set_audio_encoding_type_and_bitrate(
        game_chat_audio_encoding_type_and_bitrate::silk_16_kilobits_per_second
        );

    std::weak_ptr<ChatIntegrationLayer> weakPtrToThis = shared_from_this();

    // If the user is removed from the system, remove them from chat
    m_signOutToken = User::SignOutStarted += ref new EventHandler<SignOutStartedEventArgs^>(
        [weakPtrToThis](Platform::Object^, SignOutStartedEventArgs^ eventArgs)
    {
        std::shared_ptr<ChatIntegrationLayer> This(weakPtrToThis.lock());

        if (This == nullptr)
        {
            return;
        }

        This->RemoveLocalUser(eventArgs->User->XboxUserId->Data());
    });

    m_initialized = true;
}


void ChatIntegrationLayer::Shutdown()
{
    DebugTrace("ChatIntegrationLayer::Shutdown()\n");

    concurrency::critical_section::scoped_lock lock(m_lock);

    if (m_initialized)
    {
        chat_manager::singleton_instance().cleanup();
        m_consoles.clear();

        User::UserRemoved -= m_signOutToken;

        m_initialized = false;
    }
}

void ChatIntegrationLayer::OnSecureDeviceAssocationConnectionEstablished(
    Microsoft::Xbox::Samples::NetworkMesh::MeshConnection^ connection,
    std::vector<std::wstring> xuids
)
{
    DebugTrace("ChatIntegrationLayer::OnSecureDeviceAssocationConnectionEstablished()\n");

    concurrency::critical_section::scoped_lock lock(m_lock);

    if (m_initialized)
    {
        auto id = m_nextId++;

        connection->SetRemoteId(id);

        m_consoles[id] = connection;
        m_consoleUsers[id] = xuids;

        for (auto xuid : xuids)
        {
            DebugTrace("Adding remote user: %ws\n", xuid.c_str());

            chat_manager::singleton_instance().add_remote_user(
                xuid.c_str(),
                id
                );

            m_channels[xuid] = 0;
        }

        RebuildRelationshipMap();
    }
}

void ChatIntegrationLayer::ProcessStateChanges()
{
    if (!m_initialized)
    {
        return;
    }

    uint32_t count;
    game_chat_state_change_array changes;

    chat_manager::singleton_instance().start_processing_state_changes(
        &count,
        &changes
        );

    for (uint32_t i = 0; i < count; i++)
    {
        auto change = changes[i];

        switch (change->state_change_type)
        {
        case game_chat_state_change_type::communication_relationship_adjuster_changed:
            DebugTrace("GameChatManager::ProcessStateChanges() communication_relationship_adjuster_changed\n");
            break;

        case game_chat_state_change_type::text_chat_received:
            DebugTrace("GameChatManager::ProcessStateChanges() text_chat_received\n");
            HandleTextMessage(change, false);
            break;

        case game_chat_state_change_type::text_conversion_preference_changed:
            DebugTrace("GameChatManager::ProcessStateChanges() text_conversion_preference_changed\n");
            break;

        case game_chat_state_change_type::transcribed_chat_received:
            DebugTrace("GameChatManager::ProcessStateChanges() transcribed_chat_received\n");
            HandleTextMessage(change, true);
            break;
        }
    }

    chat_manager::singleton_instance().finish_processing_state_changes(changes);
}

void ChatIntegrationLayer::ProcessDataFrames()
{
    auto meshManager = Sample::Instance()->MeshManager();

    if (meshManager == nullptr || meshManager->GetMeshPacketManager() == nullptr)
    {
        return;
    }

    uint32_t dataFrameCount;
    game_chat_data_frame_array dataFrames;

    chat_manager::singleton_instance().start_processing_data_frames(
        &dataFrameCount,
        &dataFrames
        );

    if (dataFrameCount > 0)
    {
        for (uint32_t dataFrameIndex = 0; dataFrameIndex < dataFrameCount; ++dataFrameIndex)
        {
            auto dataFrame = dataFrames[dataFrameIndex];

            ComPtr<ABI::Windows::Storage::Streams::IBuffer> wrapBuffer = Make<WrapBuffer>(dataFrame->packet_buffer, dataFrame->packet_byte_count);

            for (uint32_t targetIndex = 0; targetIndex < dataFrame->target_endpoint_identifier_count; ++targetIndex)
            {
                concurrency::critical_section::scoped_lock lock(m_lock);

                auto targetIdentifier = dataFrame->target_endpoint_identifiers[targetIndex];
                auto conn = m_consoles[targetIdentifier];

                if (conn != nullptr)
                {
                    meshManager->GetMeshPacketManager()->SendChatMessage(
                        conn->GetAssociation(),
                        reinterpret_cast<IBuffer^>(wrapBuffer.Get()),
                        dataFrame->transport_requirement == game_chat_data_transport_requirement::guaranteed
                        );
                }
            }
        }
    }

    chat_manager::singleton_instance().finish_processing_data_frames(dataFrames);
}

void ChatIntegrationLayer::HandleTextMessage(
    const game_chat_state_change *change,
    bool transcribed
    )
{
    Platform::String^ xuid;
    Platform::String^ name;
    Platform::String^ message;

    if (transcribed)
    {
        auto event = static_cast<const game_chat_transcribed_chat_received_state_change*>(change);
        xuid = ref new Platform::String(event->speaker->xbox_user_id());
        message = ref new Platform::String(event->message);
    }
    else
    {
        auto event = static_cast<const game_chat_text_chat_received_state_change*>(change);
        xuid = ref new Platform::String(event->sender->xbox_user_id());
        message = ref new Platform::String(event->message);
    }

    name = Sample::Instance()->GetNameFromXuid(xuid);

    if (name->IsEmpty())
    {
        name = xuid;
    }

    DebugTrace("Text Message from %ws: %ws\n", name->Data(), message->Data());

    Windows::Xbox::UI::Accessibility::SendSpeechToTextString(
        name,
        message,
        transcribed ?
            Windows::Xbox::UI::SpeechToTextType::Voice :
            Windows::Xbox::UI::SpeechToTextType::Text
        );
}

void ChatIntegrationLayer::AddLocalUser(
    const wchar_t* user
)
{
    if (m_initialized)
    {
        concurrency::critical_section::scoped_lock lock(m_lock);

        DebugTrace("AddLocalUser: %ws\n", user);

        chat_manager::singleton_instance().add_local_user(
            user
            );

        // Set users on channel 0 by default
        m_channels[user] = 0;

        RebuildRelationshipMap();
    }
}

void
ChatIntegrationLayer::RebuildRelationshipMap()
{
    for (auto xboxUser : User::Users)
    {
        auto localUser = GetChatUserByXboxUserId(xboxUser->XboxUserId->Data());

        uint32_t chatUserCount;
        chat_user_array chatUsers;

        chat_manager::singleton_instance().get_chat_users(
            &chatUserCount,
            &chatUsers
            );

        for (uint32_t chatUserIndex = 0; chatUserIndex < chatUserCount; ++chatUserIndex)
        {
            auto chatUser = chatUsers[chatUserIndex];

            if (chatUser != localUser)
            {
                auto currentChannel = m_channels[chatUser->xbox_user_id()];
                auto relationship = game_chat_communication_relationship_flags::none;

                // You can fully participate with users in the same channel
                if (m_channels[localUser->xbox_user_id()] == currentChannel)
                {
                    relationship = c_communicationRelationshipSendAndReceiveAll;
                }

                localUser->local()->set_communication_relationship(
                    chatUser,
                    relationship
                    );
            }
        }
    }
}

void ChatIntegrationLayer::RemoveLocalUser(
    const wchar_t* xuid
    )
{
    DebugTrace("RemoveLocalUser: %ws\n", xuid);

    if (m_initialized)
    {
        chat_manager::singleton_instance().remove_user(
            GetChatUserByXboxUserId(xuid)
            );
    }
}

void ChatIntegrationLayer::RemoveRemoteConsole(
    uint64_t uniqueRemoteConsoleIdentifier
    )
{
    DebugTrace("ChatIntegrationLayer::RemoveRemoteConsole()\n");

    if (m_initialized)
    {
        concurrency::critical_section::scoped_lock lock(m_lock);

        uint32_t chatUserCount;
        chat_user_array chatUsers;

        chat_manager::singleton_instance().get_chat_users(
            &chatUserCount,
            &chatUsers
            );

        for (auto xuid : m_consoleUsers[uniqueRemoteConsoleIdentifier])
        {
            for (uint32_t chatUserIndex = 0; chatUserIndex < chatUserCount; ++chatUserIndex)
            {
                auto chatUser = chatUsers[chatUserIndex];

                if (xuid == chatUser->xbox_user_id())
                {
                    chat_manager::singleton_instance().remove_user(
                        chatUser
                        );

                    DebugTrace("Removing remote user: %ws\n", xuid.c_str());

                    break;
                }
            }
        }

        m_consoles.erase(uniqueRemoteConsoleIdentifier);
        m_consoleUsers.erase(uniqueRemoteConsoleIdentifier);
    }
}

chat_user*
ChatIntegrationLayer::GetChatUserByXboxUserId(
    const wchar_t* xuid
    )
{
    if (m_initialized)
    {
        uint32_t count;
        chat_user_array chatUsers;

        chat_manager::singleton_instance().get_chat_users(
            &count,
            &chatUsers
            );

        for (uint32_t i = 0; i < count; ++i)
        {
            auto user = chatUsers[i];

            if (std::wstring(xuid) == user->xbox_user_id())
            {
                return user;
            }
        }
    }

    return nullptr;
}

std::vector<std::wstring>
ChatIntegrationLayer::GetChatUsersXuids()
{
    auto users = std::vector<std::wstring>();

    if (m_initialized)
    {
        uint32_t chatUserCount;
        chat_user_array chatUsers;

        chat_manager::singleton_instance().get_chat_users(
            &chatUserCount,
            &chatUsers
            );

        for (uint32_t chatUserIndex = 0; chatUserIndex < chatUserCount; ++chatUserIndex)
        {
            users.push_back(chatUsers[chatUserIndex]->xbox_user_id());
        }
    }

    return users;
}

void
ChatIntegrationLayer::ChangeChannelForUser(
    const wchar_t* chatUser,
    uint8_t newChatChannelIndex
    )
{
    concurrency::critical_section::scoped_lock lock(m_lock);

    m_channels[chatUser] = newChatChannelIndex;
    RebuildRelationshipMap();
}

void
ChatIntegrationLayer::ChangeChatUserMuteState(
    const wchar_t* xuid
    )
{
    // Helper function to swap the mute state of a specific chat user
    auto chatUser = GetChatUserByXboxUserId(xuid);

    if (chatUser->local() != nullptr)
    {
        chatUser->local()->set_microphone_muted(
            !chatUser->local()->microphone_muted()
            );
    }
    else
    {
        for (auto user : User::Users)
        {
            auto localUser = GetChatUserByXboxUserId(user->XboxUserId->Data());

            localUser->local()->set_remote_user_muted(
                chatUser,
                !localUser->local()->remote_user_muted(chatUser)
                );
        }
    }
}

uint8_t
ChatIntegrationLayer::GetChannelForUser(
    const wchar_t* xuid
    )
{
    concurrency::critical_section::scoped_lock lock(m_lock);
    return m_channels[xuid];
}

void
ChatIntegrationLayer::OnIncomingChatMessage(
    Windows::Storage::Streams::IBuffer^ data,
    uint64_t id
)
{
    concurrency::critical_section::scoped_lock lock(m_lock);

    if (m_initialized)
    {
        byte *buffer;

        Utils::GetBufferBytes(data, &buffer);

        chat_manager::singleton_instance().process_incoming_data(
            id,                          // Remote console id
            data->Length,                // Buffer size
            buffer                       // Raw buffer pointer
            );
    }
}
