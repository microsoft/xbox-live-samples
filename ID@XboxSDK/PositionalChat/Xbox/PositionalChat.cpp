//--------------------------------------------------------------------------------------
// PositionalChat.cpp
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "pch.h"

#include "PositionalChat.h"
#include "ATGColors.h"

extern void ExitSample();

using namespace DirectX;
using namespace Windows::Xbox::Input;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Xbox::Multiplayer;
using namespace Windows::Xbox::System;
using namespace Microsoft::Xbox::Samples::NetworkMesh;
using namespace Microsoft::Xbox::Services;
using namespace Microsoft::Xbox::Services::Multiplayer;
using namespace Microsoft::Xbox::Services::Multiplayer::Manager;
using namespace Windows::Data::Json;
using namespace xbox::services::game_chat_2;

using Microsoft::WRL::ComPtr;

Sample* Sample::s_this = nullptr;

Sample::Sample() :
    m_frame(0),
    m_inLobby(true),
    m_inChat(false),
    m_selectPlayer(false),
    m_searching(false),
    m_refreshTimer(1.0f),
    m_needNames(false),
    m_loadingNames(false),
    m_meshInitialized(false),
    m_leavingToJoin(false),
    m_adjustPosition(false)
{
    s_this = this;
    m_deviceResources = std::make_unique<DX::DeviceResources>();

    // Set up UI using default fonts and colors.
    ATG::UIConfig uiconfig;
    m_ui = std::make_shared<ATG::UIManager>(uiconfig);
}

// Initialize the Direct3D resources required to run.
void Sample::Initialize(IUnknown* window)
{
    m_gamePad = std::make_unique<GamePad>();

    m_partyActive = PartyChat::IsPartyChatActive;
    m_partySuppressed = PartyChat::IsPartyChatSuppressed;

    PartyChat::IsPartyChatActiveChanged += ref new EventHandler<bool>([this](Platform::Object^, bool state)
    {
        m_partyActive = state;
    });

    PartyChat::IsPartyChatSuppressedChanged += ref new EventHandler<bool>([this](Platform::Object^, bool state)
    {
        m_partySuppressed = state;
    });

    SetupUI();

    m_deviceResources->SetWindow(window);
    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    // Hint where we'd like the STT window
    Windows::Xbox::UI::Accessibility::SetSpeechToTextPositionHint(
        Windows::Xbox::UI::SpeechToTextPositionHint::MiddleRight
        );

    auto users = User::Users;
    for (auto user : users)
    {
        if (user->IsSignedIn)
        {
            m_context = ref new XboxLiveContext(user);
            break;
        }
    }

    m_multiplayerManager = MultiplayerManager::SingletonInstance;
    m_multiplayerManager->Initialize(GAME_MULTIPLAYER_SESSION_TEMPLATE_NAME);

    m_userList->LoadImages(m_deviceResources->GetD3DDevice());
    m_chatList->LoadImages(m_deviceResources->GetD3DDevice());
    m_playerList->LoadImages(m_deviceResources->GetD3DDevice());

    UpdateControllerUIState();

    m_ui->FindPanel<ATG::Overlay>(200)->Show();

    m_reverbIndex = 0;
    m_fListenerAngle = 0.0f;
    m_fEmitterAngle = X3DAUDIO_PI;

    RECT drawspace = m_deviceResources->GetOutputSize();
    
    m_gameAudio = new GameAudio();
    m_gameAudio->Init(drawspace.right, drawspace.bottom);
}

#pragma region Frame Update
// Executes basic render loop.
void Sample::Tick()
{
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Frame %I64u", m_frame);

    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();

    PIXEndEvent();
    m_frame++;
}

// Updates the world.
void Sample::Update(DX::StepTimer const& timer)
{
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update");

    float elapsedTime = float(timer.GetElapsedSeconds());

    if (m_refreshTimer <= 0.0f && m_needNames)
    {
        LoadUserDisplayNames();
    }
    else
    {
        m_refreshTimer -= elapsedTime;
    }

    UpdateUIState();
    UpdateControllerUIState();

    if (m_activationArgs != nullptr)
    {
        if (m_inLobby)
        {
            JoinSession();
        }
        else if (!m_leavingToJoin)
        {
            m_leavingToJoin = true;

            if (m_inChat)
            {
                LeaveChat();
                GetChatIntegrationLayer()->ProcessDataFrames();
            }

            auto users = User::Users;
            for (auto user : users)
            {
                m_multiplayerManager->LobbySession->RemoveLocalUser(user);
            }
        }
    }

    auto leave = false;
    auto exiting = false;

    for (unsigned x = 0; x < MAXUSERS; x++)
    {
        auto pad = m_gamePad->GetState(x);
        if (pad.IsConnected())
        {
            auto padUser = GamepadFromPadId(m_gamePad->GetCapabilities(x).id)->User;

            m_gamePadButtons.Update(pad);
            if (padUser == nullptr || !padUser->IsSignedIn)
            {
                if (!m_inLobby && m_gamePadButtons.y != GamePad::ButtonStateTracker::PRESSED)
                {
                    // A non-signed in user can only attempt to sign in
                    continue;
                }
            }

            if (m_inLobby)
            {
                if (m_gamePadButtons.y == GamePad::ButtonStateTracker::PRESSED)
                {
                    BindControllerToUser(GamepadFromPadId(m_gamePad->GetCapabilities(x).id));
                }
                else if (m_gamePadButtons.a == GamePad::ButtonStateTracker::PRESSED)
                {
                    if (m_context == nullptr)
                    {
                        m_context = ref new XboxLiveContext(padUser);
                    }

                    JoinSession();
                }
                else if (m_gamePadButtons.b == GamePad::ButtonStateTracker::PRESSED)
                {
                    ExitSample();
                    exiting = true;
                }
                else if (m_gamePadButtons.x == GamePad::ButtonStateTracker::PRESSED)
                {
                    if (m_context == nullptr)
                    {
                        m_context = ref new XboxLiveContext(padUser);
                    }

                    SearchJoinableUsers(GamepadFromPadId(m_gamePad->GetCapabilities(x).id)->User);
                }
            }
            else if (m_selectPlayer)
            {
                if (m_gamePadButtons.b == GamePad::ButtonStateTracker::PRESSED)
                {
                    leave = true;
                }
                else if (m_gamePadButtons.x == GamePad::ButtonStateTracker::PRESSED)
                {
                    SearchJoinableUsers(padUser);
                }
            }
            else
            {
                if (m_gamePadButtons.b == GamePad::ButtonStateTracker::PRESSED)
                {
                    auto users = User::Users;
                    for (auto user : users)
                    {
                        m_multiplayerManager->LobbySession->RemoveLocalUser(user);
                    }

                    if (m_inChat)
                    {
                        LeaveChat();
                    }
                }
                else if (m_gamePadButtons.y == GamePad::ButtonStateTracker::PRESSED)
                {
                    SendTextMessage(padUser->XboxUserId);
                }
                else if (m_gamePadButtons.menu == GamePad::ButtonStateTracker::PRESSED)
                {
                    InviteUsers(padUser);
                }
                else if (m_gamePadButtons.leftShoulder == GamePad::ButtonStateTracker::PRESSED)
                {
                    ChangeChannel(padUser, -1);
                }
                else if (m_gamePadButtons.rightShoulder == GamePad::ButtonStateTracker::PRESSED)
                {
                    ChangeChannel(padUser, 1);
                }
                else if (m_gamePadButtons.rightStick == GamePad::ButtonStateTracker::PRESSED)
                {
                    Windows::Xbox::Multiplayer::PartyChat::IsPartyChatSuppressed = !m_partySuppressed;
                }
                else if (m_gamePadButtons.view == GamePad::ButtonStateTracker::PRESSED)
                {
                    m_adjustPosition = !m_adjustPosition;
                }
                else if (m_gamePadButtons.dpadUp == GamePad::ButtonStateTracker::PRESSED)
                {
                    m_reverbIndex++;

                    if (m_reverbIndex >= NUM_PRESETS - 1)
                    {
                        m_reverbIndex = 0;
                    }

                    m_gameAudio->SetReverb(m_reverbIndex);
                }
                else if (m_gamePadButtons.dpadDown == GamePad::ButtonStateTracker::PRESSED)
                {
                    m_reverbIndex--;

                    if (m_reverbIndex < 0)
                    {
                        m_reverbIndex = NUM_PRESETS - 1;
                    }

                    m_gameAudio->SetReverb(m_reverbIndex);
                }
                else if (m_gamePadButtons.dpadLeft == GamePad::ButtonStateTracker::PRESSED ||
                    m_gamePadButtons.dpadRight == GamePad::ButtonStateTracker::PRESSED)
                {
                    //Adjust what direction the emitter is facing
                    float fShoulders = 0;
                    if (m_gamePadButtons.dpadLeft == GamePad::ButtonStateTracker::PRESSED)
                    {
                        fShoulders -= 1.0f;
                    }

                    if (m_gamePadButtons.dpadRight == GamePad::ButtonStateTracker::PRESSED)
                    {
                        fShoulders += 1.0f;
                    }

                    m_gameAudio->AdjustEmitterAngle(fShoulders, &m_fEmitterAngle);
                }
            }

            m_gamePadButtons.Reset();
            
            m_gameAudio->Update(pad.thumbSticks.rightX * moveScalar, pad.thumbSticks.rightY * moveScalar);
        }
        else
        {
            m_gamePadButtons.Reset();
        }
    }

    auto pad = m_gamePad->GetState(0);
    if (pad.IsConnected())
    {
        concurrency::critical_section::scoped_lock lock(m_uiLock);

        m_gamePadButtons.Update(pad);

        m_ui->Update(elapsedTime, pad);
    }
    else
    {
        m_gamePadButtons.Reset();
    }

    if (leave)
    {
        m_selectPlayer = false;
        LeaveSession();
    }

    if (!exiting)
    {
        ProcessMpmMessages();
    }

    GetChatIntegrationLayer()->ProcessStateChanges();
    GetChatIntegrationLayer()->ProcessStreamStateChanges();
    GetChatIntegrationLayer()->ProcessPostDecodeBuffers();

    PIXEndEvent();
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Sample::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    // Prepare the render target to render a new frame.
    m_deviceResources->Prepare();
    Clear();

    auto context = m_deviceResources->GetD3DDeviceContext();
    PIXBeginEvent(context, PIX_COLOR_DEFAULT, L"Render");

    if (m_adjustPosition)
    {
        auto rect = m_deviceResources->GetOutputSize();
        auto safeRect = SimpleMath::Viewport::ComputeTitleSafeArea(rect.right, rect.bottom);
        XMFLOAT2 pos(float(safeRect.left), float(safeRect.top));
        wchar_t str[256] = {};

        // Draw the grid
        DrawGrid();

        // Draw listener in white
        DrawPointerCross((float)rect.right / 2, (float)rect.bottom / 2, m_fListenerAngle, ATG::Colors::White);

        // Draw emitter in green
        DrawPointerCross(m_gameAudio->EmitterX(), rect.bottom - m_gameAudio->EmitterY(), m_fEmitterAngle, ATG::Colors::Green);
        DrawCone(m_gameAudio->EmitterConeOuter(), m_gameAudio->EmitterConeInner(), m_gameAudio->EmitterX(), rect.bottom - m_gameAudio->EmitterY(), m_fEmitterAngle, ATG::Colors::Green, DISTANCE_SCALAR);
        DrawCircle(m_gameAudio->EmitterX(), rect.bottom - m_gameAudio->EmitterY(), DISTANCE_SCALAR / 2);
        DrawCircle(m_gameAudio->EmitterX(), rect.bottom - m_gameAudio->EmitterY(), DISTANCE_SCALAR);

        m_spriteBatch->Begin();

        swprintf_s(str, 256, L"Emitter(%f, %f) Angle: %f rad", m_gameAudio->EmitterX(), m_gameAudio->EmitterY(), m_fEmitterAngle);
        m_font->DrawString(m_spriteBatch.get(), str, pos, ATG::Colors::White);
        pos.y += 30;

        swprintf_s(str, 256, L"Reverb: %s", m_gameAudio->GetReverbName(m_reverbIndex).data());
        m_font->DrawString(m_spriteBatch.get(), str, pos, ATG::Colors::White);
        pos.y += 30;

        m_spriteBatch->End();
    }
    else
    {
        concurrency::critical_section::scoped_lock lock(m_uiLock);

        m_ui->Render();

        if (!m_inLobby && !m_selectPlayer)
        {
            m_console->Render();
        }
    }

    PIXEndEvent(context);

    // Show the new frame.
    PIXBeginEvent(context, PIX_COLOR_DEFAULT, L"Present");
    m_deviceResources->Present();
    m_graphicsMemory->Commit();
    PIXEndEvent(context);
}

// Helper method to clear the back buffers.
void Sample::Clear()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    PIXBeginEvent(context, PIX_COLOR_DEFAULT, L"Clear");

    // Clear the views.
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, ATG::Colors::Background);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    PIXEndEvent(context);
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Sample::OnSuspending()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    context->Suspend(0);
    LeaveSession();
}

void Sample::OnResuming()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    context->Resume();
    m_timer.ResetElapsedTime();
    m_gamePadButtons.Reset();

    // Reset UI on return from suspend.
    m_ui->Reset();
}

void Sample::OnProtocolActivation(Windows::ApplicationModel::Activation::IProtocolActivatedEventArgs ^ args)
{
    m_activationArgs = args;
}
#pragma endregion

#pragma region Helpers
//--------------------------------------------------------------------------------------
// Name: ConvertToScreenSpace()
// Desc: Draws a grey grid
//--------------------------------------------------------------------------------------
void Sample::ConvertToScreenSpace(float inX, float inY, float* outX, float* outY)
{
    RECT drawspace = m_deviceResources->GetOutputSize();

    *outX = 2 * inX / drawspace.right - 1.f;
    *outY = 2 * inY / drawspace.bottom - 1.f;
}

//--------------------------------------------------------------------------------------
// Name: DrawGrid()
// Desc: Draws a grey grid
//--------------------------------------------------------------------------------------
void Sample::DrawGrid()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    PIXBeginEvent(context, PIX_COLOR_DEFAULT, L"Draw grid");

    context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(m_states->DepthNone(), 0);
    context->RSSetState(m_states->CullNone());

    m_batchEffect->Apply(context);

    context->IASetInputLayout(m_batchInputLayout.Get());

    m_batch->Begin();

    RECT drawspace = m_deviceResources->GetOutputSize();

    //Vertical lines
    XMVECTORF32 topPoint;
    XMVECTORF32 bottomPoint;
    VertexPositionColor top;
    VertexPositionColor bottom;
    float x, y;

    for (float i = 0; i<drawspace.right; i += 100.f)
    {
        ConvertToScreenSpace(i, 0, &x, &y);
        topPoint = { x, -1.f, 0.f };
        bottomPoint = { x, 1.f, 0.f };
        top = VertexPositionColor(topPoint, ATG::Colors::OffWhite);
        bottom = VertexPositionColor(bottomPoint, ATG::Colors::OffWhite);
        m_batch->DrawLine(top, bottom);
    }

    //Horizontal lines
    for (float i = 0; i<drawspace.bottom; i += 100.f)
    {
        ConvertToScreenSpace(0, i, &x, &y);
        topPoint = { -1.f, y, 0.f };
        bottomPoint = { 1.f, y, 0.f };
        top = VertexPositionColor(topPoint, ATG::Colors::OffWhite);
        bottom = VertexPositionColor(bottomPoint, ATG::Colors::OffWhite);
        m_batch->DrawLine(top, bottom);
    }

    m_batch->End();

    PIXEndEvent(context);
}

//--------------------------------------------------------------------------------------
// Name: DrawCircle()
// Desc: Draws a circle at the given position and given size
//--------------------------------------------------------------------------------------
void Sample::DrawCircle(float x, float y, float radius)
{
    //Circle has a default radius of CIRCLE_RADIUS
    float adjustedRadius = radius * radius / CIRCLE_RADIUS;
    RECT drawspace;

    RECT screenspace = m_deviceResources->GetOutputSize();

    drawspace.bottom = (LONG)(screenspace.bottom - y + adjustedRadius);
    drawspace.left = (LONG)(x - adjustedRadius);
    drawspace.right = (LONG)(x + adjustedRadius);
    drawspace.top = (LONG)(screenspace.bottom - y - adjustedRadius);

    m_spriteBatch->Begin();
    m_spriteBatch->Draw(m_circle.Get(), drawspace, ATG::Colors::White);
    m_spriteBatch->End();
}

//--------------------------------------------------------------------------------------
// Name: DrawCone()
// Desc: Draws a cone
//--------------------------------------------------------------------------------------
void Sample::DrawCone(float OuterAngle, float InnerAngle, float x, float y, float fAngle, XMVECTOR vOutlineColor, UINT size)
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    PIXBeginEvent(context, PIX_COLOR_DEFAULT, L"Draw cone");

    context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(m_states->DepthNone(), 0);
    context->RSSetState(m_states->CullNone());

    m_batchEffect->Apply(context);

    context->IASetInputLayout(m_batchInputLayout.Get());

    float outerX = sin(OuterAngle / 2) * size;
    float innerX = sin(InnerAngle / 2) * size;
    float outerY = -cos(OuterAngle / 2) * size;
    float innerY = -cos(InnerAngle / 2) * size;

    XMVECTOR v[4];
    //Outside
    v[0] = XMVectorSet(-outerX, outerY, 0.0f, 1.0f);
    v[1] = XMVectorSet(outerX, outerY, 0.0f, 1.0f);
    //Inside
    v[2] = XMVectorSet(-innerX, innerY, 0.0f, 1.0f);
    v[3] = XMVectorSet(innerX, innerY, 0.0f, 1.0f);

    XMVECTOR vout[4];
    XMVECTOR vZero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

    XMMATRIX finalTransform = XMMatrixTransformation2D(vZero, 0, XMVectorSet(1, 1, 1.0f, 1.0f), vZero, -fAngle, XMVectorSet(x, y, 0.0f, 1.0f));

    vout[0] = XMVector4Transform(v[0], finalTransform);
    vout[1] = XMVector4Transform(v[1], finalTransform);
    vout[2] = XMVector4Transform(v[2], finalTransform);
    vout[3] = XMVector4Transform(v[3], finalTransform);
        
    m_batch->Begin();
        
    float newX, newY;
    ConvertToScreenSpace(x, y, &newX, &newY);

    XMVECTORF32 centerPoint = {newX, newY, 0.f};
    VertexPositionColor center(centerPoint, vOutlineColor);
    VertexPositionColor start;

    for (int i = 0; i < 4; i++)
    {
        ConvertToScreenSpace(XMVectorGetX(vout[i]), XMVectorGetY(vout[i]), &newX, &newY);
        vout[i] = XMVectorSet(newX, newY, 0.0f, 1.0f);
        start = VertexPositionColor(vout[i], vOutlineColor);
        m_batch->DrawLine(start, center);
    }

    m_batch->End();

    PIXEndEvent(context);
}


//--------------------------------------------------------------------------------------
// Name: DrawPointerCross()
// Desc: Draws the cross that indicates where audio listener/emitter are
//--------------------------------------------------------------------------------------
void Sample::DrawPointerCross(float x, float y, float fAngle, XMVECTOR vOutlineColor)
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    PIXBeginEvent(context, PIX_COLOR_DEFAULT, L"Draw pointer cross");

    context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(m_states->DepthNone(), 0);
    context->RSSetState(m_states->CullNone());

    m_batchEffect->Apply(context);

    context->IASetInputLayout(m_batchInputLayout.Get());

    //Build the shape at the origin
    XMVECTOR v[11];
    v[0] = XMVectorSet(-1.5f, -8.5f, 0.0f, 1.0f);
    v[1] = XMVectorSet(0.5f, -8.5f, 0.0f, 1.0f);
    v[2] = XMVectorSet(0.5f, 7.5f, 0.0f, 1.0f);
    v[3] = XMVectorSet(-1.5f, 7.5f, 0.0f, 1.0f);

    v[4] = XMVectorSet(-8.5f, -1.5f, 0.0f, 1.0f);
    v[5] = XMVectorSet(7.5f, -1.5f, 0.0f, 1.0f);
    v[6] = XMVectorSet(7.5f, 0.5f, 0.0f, 1.0f);
    v[7] = XMVectorSet(-8.5f, 0.5f, 0.0f, 1.0f);

    v[8] = XMVectorSet(0.0f, -15.0f, 0.0f, 1.0f);
    v[9] = XMVectorSet(-8.0f, -8.5f, 0.0f, 1.0f);
    v[10] = XMVectorSet(8.0f, -8.5f, 0.0f, 1.0f);

    XMVECTOR vout[11];
    XMVECTOR vZero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

    //Draw the primary arrow
    XMMATRIX finalTransform = XMMatrixTransformation2D(vZero, 0, XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f), vZero, fAngle, XMVectorSet(x, y, 0.0f, 1.0f));

    vout[0] = XMVector4Transform(v[0], finalTransform);
    vout[1] = XMVector4Transform(v[1], finalTransform);
    vout[2] = XMVector4Transform(v[2], finalTransform);
    vout[3] = XMVector4Transform(v[3], finalTransform);
    vout[4] = XMVector4Transform(v[4], finalTransform);
    vout[5] = XMVector4Transform(v[5], finalTransform);
    vout[6] = XMVector4Transform(v[6], finalTransform);
    vout[7] = XMVector4Transform(v[7], finalTransform);
    vout[8] = XMVector4Transform(v[8], finalTransform);
    vout[9] = XMVector4Transform(v[9], finalTransform);
    vout[10] = XMVector4Transform(v[10], finalTransform);

    m_batch->Begin();

    VertexPositionColor points[11];
    float newX, newY;

    for (int i = 0; i < 11; i++)
    {
        ConvertToScreenSpace(XMVectorGetX(vout[i]), XMVectorGetY(vout[i]), &newX, &newY);
        vout[i] = XMVectorSet(newX, newY, 0.0f, 1.0f);
        points[i] = VertexPositionColor(vout[i], vOutlineColor);
    }

    //Forward line
    m_batch->DrawLine(points[0], points[1]);
    m_batch->DrawLine(points[1], points[2]);
    m_batch->DrawLine(points[2], points[3]);
    m_batch->DrawLine(points[3], points[0]);

    //Horizontal line
    m_batch->DrawLine(points[4], points[5]);
    m_batch->DrawLine(points[5], points[6]);
    m_batch->DrawLine(points[6], points[7]);
    m_batch->DrawLine(points[7], points[4]);

    //Triangle
    m_batch->DrawLine(points[8], points[9]);
    m_batch->DrawLine(points[8], points[10]);
    m_batch->DrawLine(points[9], points[10]);

    m_batch->End();
}

Platform::String ^ Sample::GetNameFromXuid(Platform::String ^ xuid)
{
    concurrency::critical_section::scoped_lock lock(m_stateLock);

    auto it = m_xuidToGamertag.find(xuid);
    if (it != std::end(m_xuidToGamertag))
    {
        return (*it).second;
    }

    ReloadDisplayNames();

    return xuid;
}

void Sample::ReloadDisplayNames()
{
    m_needNames = true;
}

void Sample::LogToConsole(Platform::String ^ val)
{
    if (m_console)
    {
        m_console->Write(val->Data());
    }
}

Windows::Xbox::Input::IGamepad^ Sample::GamepadFromPadId(uint64_t id)
{
    IVectorView<IGamepad^>^ gamepads = Gamepad::Gamepads;

    for (unsigned i = 0; i < gamepads->Size; i++)
    {
        auto gamepad = gamepads->GetAt(i);

        if (gamepad->Id == id)
        {
            return gamepad;
        }
    }

    return nullptr;
}

std::wstring Sample::FormatNumber(double val)
{
    if (val == 100000.0)
    {
        val = 0.0;
    }

    wchar_t buffer[32] = {};
    int intval = (int)val;

    swprintf_s(buffer, 32, L"%02d.%02d", intval, (int)((val - (double)intval) * 100));

    return std::wstring(buffer);
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Sample::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();

    m_graphicsMemory = std::make_unique<GraphicsMemory>(device, m_deviceResources->GetBackBufferCount());

    m_spriteBatch = std::make_unique<SpriteBatch>(context);

    m_states = std::make_unique<CommonStates>(device);

    m_batchEffect = std::make_unique<BasicEffect>(device);
    m_batchEffect->SetVertexColorEnabled(true);

    {
        void const* shaderByteCode;
        size_t byteCodeLength;

        m_batchEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

        DX::ThrowIfFailed(
            device->CreateInputLayout(VertexPositionColor::InputElements,
                VertexPositionColor::InputElementCount,
                shaderByteCode, byteCodeLength,
                m_batchInputLayout.ReleaseAndGetAddressOf())
        );
    }

    m_console = std::make_unique <DX::TextConsole>(context, L"Assets\\Consolas_8.spritefont");
    m_console->SetDebugOutput(true);

    // Let UI create Direct3D resources.
    m_ui->RestoreDevice(context);

    m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(m_deviceResources->GetD3DDeviceContext());

    m_font = std::make_unique<SpriteFont>(device, L"SegoeUI_18.spritefont");

    CreateDDSTextureFromFile(device, L"Assets\\circle.DDS", nullptr, m_circle.ReleaseAndGetAddressOf());
}

// Allocate all memory resources that change on a window SizeChanged event.
void Sample::CreateWindowSizeDependentResources()
{
    RECT fullscreen = m_deviceResources->GetOutputSize();

    // Let UI know the size of our rendering viewport.
    m_ui->SetWindow(fullscreen);

    RECT console = {};

    console.top = 247;
    console.left = 795;
    console.bottom = console.top + 641;
    console.right = console.left + 626;

    m_console->SetWindow(console);
}
#pragma endregion

#pragma region GUI Setup and Drawing
void Sample::SetupUI()
{
    m_ui->LoadLayout(L".\\Assets\\Layout.csv", L".\\Assets");

    // Setup user list repeaters
    auto loc = POINT{ 169, 249 };
    auto pos = SIZE{ 610, 76 };

    // UserList is the 'local lobby' user list
    m_userList = std::make_unique<UserRepeater>(m_ui, loc, pos, 8000);
    m_userList->SetReadOnly(true);

    // ChatList is the active session user list
    m_chatList = std::make_unique<UserRepeater>(m_ui, loc, pos, 9000);
    m_chatList->SetSelectedCallback([this](unsigned index)
    {
        // Mute the selected user
        auto players = GetChatIntegrationLayer()->GetChatUsersXuids();

        if (players.size() > index)
        {
            GetChatIntegrationLayer()->ChangeChatUserMuteState(players.at(index).c_str());
        }
    });

    // PlayerList is for joining friends
    m_playerList = std::make_unique<UserRepeater>(m_ui, loc, pos, 7000);
    m_playerList->SetSelectedCallback([this](unsigned index)
    {
        // Attempt to join the selected user's session
        m_selectPlayer = false;

        m_joinXuid = m_joinableXuids[index];
        JoinSession();
    });

    // Populate each list with empty items to start
    auto users = std::vector<std::shared_ptr<UserListItem>>();

    for (auto x = 0; x < MAXUSERS; x++)
    {
        users.push_back(std::make_shared<UserListItem>());
    }

    m_userList->GenerateList(200, users, 2);
    m_chatList->GenerateList(300, users, 2);
    m_playerList->GenerateList(400, users, 2);
}

void Sample::UpdateControllerUIState()
{
    // Update the current user list items

    auto users = std::vector<std::shared_ptr<UserListItem>>();

    if (m_inLobby)
    {
        // In the lobby look for attached gamepads
        for (auto x = 0; x < MAXUSERS; x++)
        {
            auto pad = m_gamePad->GetState(x);
            if (pad.IsConnected())
            {
                auto padId = m_gamePad->GetCapabilities(x).id;

                IVectorView<IGamepad^>^ gamepads = Gamepad::Gamepads;

                for (unsigned i = 0; i < gamepads->Size; i++)
                {
                    auto gamepad = gamepads->GetAt(i);

                    if (gamepad->Id == padId)
                    {
                        if (gamepad->User == nullptr)
                        {
                            users.push_back(std::make_shared<UserListItem>(true));
                        }
                        else
                        {
                            users.push_back(std::make_shared<UserListItem>(gamepad->User));
                        }
                        break;
                    }
                }
            }
            else
            {
                users.push_back(std::make_shared<UserListItem>());
            }
        }

        m_userList->UpdateList(users);
    }
    else if (m_selectPlayer)
    {
        // Populated via async call to Xbox Live
    }
    else
    {
        // Populate the list from the current GameChat session

        auto players = GetChatIntegrationLayer()->GetChatUsersXuids();

        for (unsigned i = 0; i < 8; i++)
        {
            if (i < players.size())
            {
                users.push_back(std::make_shared<UserListItem>(GetChatIntegrationLayer()->GetChatUserByXboxUserId(players.at(i).c_str())));
            }
            else
            {
                users.push_back(std::make_shared<UserListItem>());
            }
        }

        m_chatList->UpdateList(users);
    }
}

void Sample::UpdateUIState()
{
    // Update the right panel view based on user state
    concurrency::critical_section::scoped_lock lock(m_uiLock);

    if (m_partyActive && !m_partySuppressed)
    {
        m_ui->FindControl<ATG::TextLabel>(300, 320)->SetText(L"PartyChat has microphone focus");
        m_ui->FindControl<ATG::TextLabel>(300, 320)->SetForegroundColor({ 1.000000000f, 0.000000000f, 0.000000000f, 1.000000000f });
    }
    else
    {
        m_ui->FindControl<ATG::TextLabel>(300, 320)->SetText(L"GameChat has microphone focus");
        m_ui->FindControl<ATG::TextLabel>(300, 320)->SetForegroundColor({ 0.000000000f, 0.501960814f, 0.000000000f, 1.000000000f });
    }
}
#pragma endregion

#pragma region UI Actions
void Sample::BindControllerToUser(Windows::Xbox::Input::IGamepad ^ pad)
{
    DebugTrace("Sample::BindControllerToUser()\n");

    concurrency::create_task(
        Windows::Xbox::UI::SystemUI::ShowAccountPickerAsync(
            nullptr,
            Windows::Xbox::UI::AccountPickerOptions::None
        )
    ).then([this](concurrency::task<Windows::Xbox::UI::AccountPickerResult^> t)
    {
        try
        {
            t.get();

            if (m_context == nullptr)
            {
                auto users = User::Users;
                for (auto user : users)
                {
                    if (user->IsSignedIn)
                    {
                        m_context = ref new XboxLiveContext(user);
                        break;
                    }
                }
            }
        }
        catch (...)
        {
        }
    });
}

void Sample::InviteUsers(Windows::Xbox::System::User^ user)
{
    DebugTrace("Sample::InviteUsers()\n");

    m_multiplayerManager->LobbySession->InviteFriends(user, "", "");
}

void Sample::SearchJoinableUsers(Windows::Xbox::System::User^ user)
{
    DebugTrace("Sample::SearchJoinableUsers()\n");

    if (m_searching)
    {
        return;
    }

    m_searching = true;
    m_inLobby = false;
    m_selectPlayer = true;

    m_ui->FindPanel<ATG::Overlay>(400)->Show();
    m_ui->FindControl<ATG::TextLabel>(400, 102)->SetText(L"Looking for sessions...");

    m_xuidToHandle.clear();

    auto users = std::vector<std::shared_ptr<UserListItem>>();

    for (auto x = 0; x < MAXUSERS; x++)
    {
        users.push_back(std::make_shared<UserListItem>());
    }

    // Clear out the list first
    m_playerList->UpdateList(users);

    concurrency::create_task(
        m_context->MultiplayerService->GetActivitiesForSocialGroupAsync(
            m_context->AppConfig->ServiceConfigurationId,
            user->XboxUserId,
            Social::SocialGroupConstants::People
        )
    ).then([this](concurrency::task<Windows::Foundation::Collections::IVectorView<Multiplayer::MultiplayerActivityDetails^>^> t)
    {
        try
        {
            auto details = t.get();
            auto list = ref new Platform::Collections::Vector<Platform::String^>();

            for (auto detail : details)
            {
                bool joinableSession = detail->TitleId == m_context->AppConfig->TitleId
                    && detail->MembersCount < detail->MaxMembersCount
                    && !detail->Closed
                    && detail->JoinRestriction != Multiplayer::MultiplayerSessionRestriction::Local
                    && detail->Visibility == Multiplayer::MultiplayerSessionVisibility::Open;

                if (joinableSession)
                {
                    list->Append(detail->OwnerXboxUserId);
                    m_joinableXuids.push_back(detail->OwnerXboxUserId);
                    m_xuidToHandle[detail->OwnerXboxUserId] = detail->HandleId;
                }
            }

            DebugTrace("Found %d available session(s).\n", list->Size);

            if (list->Size > 0)
            {
                return concurrency::create_task(m_context->ProfileService->GetUserProfilesAsync(list->GetView()));
            }
            else
            {
                return concurrency::create_task([] () -> IVectorView<Social::XboxUserProfile^>^ { return nullptr; });
            }
        }
        catch (Platform::Exception^ ex)
        {
            return concurrency::create_task([]() -> IVectorView<Social::XboxUserProfile^>^ { return nullptr; });
        }
    }).then([this](concurrency::task<IVectorView<Social::XboxUserProfile^>^> t)
    {
        try
        {
            // Update the ui list based on the profile results

            auto profiles = t.get();
            auto users = std::vector<std::shared_ptr<UserListItem>>();

            for (unsigned i = 0; i < MAXUSERS; i++)
            {
                if (profiles != nullptr && i < profiles->Size)
                {
                    users.push_back(std::make_shared<UserListItem>(std::wstring(profiles->GetAt(i)->ApplicationDisplayName->Data())));
                }
                else
                {
                    users.push_back(std::make_shared<UserListItem>());
                }
            }

            if (profiles != nullptr)
            {
                m_ui->FindControl<ATG::TextLabel>(400, 102)->SetText(L"Select session to join");
            }
            else
            {
                m_ui->FindControl<ATG::TextLabel>(400, 102)->SetText(L"No sessions found");
            }

            m_playerList->UpdateList(users);
        }
        catch(Platform::Exception^ ex)
        {
            m_ui->FindControl<ATG::TextLabel>(400, 102)->SetText(L"Unable to get profiles");
        }

        m_searching = false;
    });
}

void Sample::SetupChat()
{
    DebugTrace("Sample::SetupChat()\n");

    GetChatIntegrationLayer()->Initialize();

    // Add all local users to gamechat
    auto list = User::Users;

    for (unsigned x = 0; x < list->Size && x < MAXUSERS; x++)
    {
        GetChatIntegrationLayer()->AddLocalUser(list->GetAt(x)->XboxUserId->Data());
    }

    m_inChat = true;
}

void Sample::LeaveChat()
{
    DebugTrace("Sample::LeaveChat()\n");

    auto list = User::Users;

    for (unsigned x = 0; x < list->Size && x < MAXUSERS; x++)
    {
        GetChatIntegrationLayer()->RemoveLocalUser(list->GetAt(x)->XboxUserId->Data());
    }

    GetChatIntegrationLayer()->ProcessDataFrames();
    GetChatIntegrationLayer()->Shutdown();
    ShutdownMeshManager();

    m_inChat = false;
}

void Sample::LoadUserDisplayNames()
{
    if (m_context == nullptr || m_needNames == false || m_loadingNames == true)
    {
        return;
    }

    DebugTrace("Sample::LoadUserDisplayNames()\n");

    m_loadingNames = true;

    auto list = ref new Platform::Collections::Vector<Platform::String^>();

    for (auto user : GetChatIntegrationLayer()->GetChatUsersXuids())
    {
        list->Append(ref new Platform::String(user.c_str()));
    }

    concurrency::create_task(
        m_context->ProfileService->GetUserProfilesAsync(
            list->GetView()
            )
    ).then([this](concurrency::task<IVectorView<Social::XboxUserProfile^>^> t)
    {
        try
        {
            auto friendProfiles = t.get();

            concurrency::critical_section::scoped_lock lock(m_stateLock);

            m_xuidToGamertag.clear();

            for (const auto& friendProfile : friendProfiles)
            {
                m_xuidToGamertag[friendProfile->XboxUserId] = friendProfile->GameDisplayName;
            }

            auto users = User::Users;
            for (auto user : users)
            {
                m_xuidToGamertag[user->XboxUserId] = user->DisplayInfo->GameDisplayName;
            }

        }
        catch(Platform::Exception^ ex)
        {
            DebugTrace("GetUserProfilesForSocialGroupAsync failed: 0x%x\n", ex->HResult);
        }

        m_refreshTimer = 1.0f;
        m_loadingNames = false;
        m_needNames = false;
    });
}

void Sample::JoinSession()
{
    DebugTrace("Sample::JoinSession()\n");

    auto users = User::Users;
    for (auto user : users)
    {
        m_multiplayerManager->LobbySession->AddLocalUser(user);
        m_multiplayerManager->LobbySession->SetLocalMemberConnectionAddress(
            user,
            Windows::Networking::XboxLive::XboxLiveDeviceAddress::GetLocal()->GetSnapshotAsBase64(),
            nullptr
        );
    }

    if (m_activationArgs != nullptr)
    {
        // Join based on protocol activation
        m_multiplayerManager->JoinLobby(m_activationArgs, User::Users);

        m_activationArgs = nullptr;
    }
    else if (!m_joinXuid->IsEmpty())
    {
        // Join a friend's session
        m_multiplayerManager->JoinLobby(m_xuidToHandle[m_joinXuid], User::Users);

        m_joinXuid = nullptr;
    }

    m_inLobby = false;
    m_ui->FindPanel<ATG::Overlay>(300)->Show();
}

void Sample::LeaveSession()
{
    DebugTrace("Sample::LeaveSession()\n");

    m_inLobby = true;
    m_ui->FindPanel<ATG::Overlay>(200)->Show();
}

void Sample::SendTextMessage(Platform::String^ xuid)
{
    auto chatUser = GetChatIntegrationLayer()->GetChatUserByXboxUserId(xuid->Data());

    concurrency::create_task(
        Windows::Xbox::UI::SystemUI::ShowVirtualKeyboardAsync(
            L"This is a text message",
            L"Send Text Message",
            L"Enter text to send:",
            Windows::Xbox::UI::VirtualKeyboardInputScope::Default
        )
    ).then([chatUser](Platform::String^ text)
    {
        chatUser->local()->send_chat_text(text->Data());

        if (chatUser->local()->text_to_speech_conversion_preference_enabled())
        {
            chatUser->local()->synthesize_text_to_speech(text->Data());
        }
    });
}

void Sample::ChangeChannel(Windows::Xbox::System::User^ user, int updown)
{
    auto channel = GetChatIntegrationLayer()->GetChannelForUser(user->XboxUserId->Data());
    auto nchan = channel + updown;

    if (nchan > 2) nchan = 0;
    if (nchan < 0) nchan = 2;

    auto chan = ref new Platform::String();

    chan += nchan;

    m_multiplayerManager->LobbySession->SetLocalMemberProperties(
        user,
        L"channel",
        chan,
        nullptr
        );

    GetChatIntegrationLayer()->ChangeChannelForUser(
        user->XboxUserId->Data(),
        static_cast<uint8>(nchan)
        );
}

#pragma endregion

void Sample::ProcessMpmMessages()
{
    auto queue = m_multiplayerManager->DoWork();
    for (auto item : queue)
    {
        switch (item->EventType)
        {
        case MultiplayerEventType::ArbitrationComplete:
            DebugTrace("MultiplayerEventType::ArbitrationComplete\n");
            break;

        case MultiplayerEventType::ClientDisconnectedFromMultiplayerService:
            DebugTrace("MultiplayerEventType::ClientDisconnectedFromMultiplayerService\n");

            if (m_inChat)
            {
                LeaveChat();
            }
            LeaveSession();

            break;

        case MultiplayerEventType::FindMatchCompleted:
            DebugTrace("MultiplayerEventType::FindMatchCompleted\n");
            break;

        case MultiplayerEventType::HostChanged:
            DebugTrace("MultiplayerEventType::HostChanged\n");
            break;

        case MultiplayerEventType::InviteSent:
            DebugTrace("MultiplayerEventType::InviteSent\n");
            break;

        case MultiplayerEventType::JoinabilityStateChanged:
            DebugTrace("MultiplayerEventType::JoinabilityStateChanged\n");
            break;

        case MultiplayerEventType::JoinGameCompleted:
            DebugTrace("MultiplayerEventType::JoinGameCompleted\n");
            break;

        case MultiplayerEventType::JoinLobbyCompleted:
        {
            DebugTrace("MultiplayerEventType::JoinLobbyCompleted\n");

            for (auto member : m_multiplayerManager->LobbySession->Members)
            {
                if (member->IsLocal)
                {
                    InitializeMeshManager(static_cast<BYTE>(member->MemberId));
                    SetupChat();
                    break;
                }
            }

            break;
        }

        case MultiplayerEventType::LeaveGameCompleted:
            DebugTrace("MultiplayerEventType::LeaveGameCompleted\n");
            break;

        case MultiplayerEventType::LocalMemberConnectionAddressWriteCompleted:
            DebugTrace("MultiplayerEventType::LocalMemberConnectionAddressWriteCompleted\n");
            break;

        case MultiplayerEventType::LocalMemberPropertyWriteCompleted:
            DebugTrace("MultiplayerEventType::LocalMemberPropertyWriteCompleted\n");
            break;

        case MultiplayerEventType::MemberJoined:
            DebugTrace("MultiplayerEventType::MemberJoined\n");
            break;

        case MultiplayerEventType::MemberLeft:
            DebugTrace("MultiplayerEventType::MemberLeft\n");
            break;

        case MultiplayerEventType::MemberPropertyChanged:
        {
            DebugTrace("MultiplayerEventType::MemberPropertyChanged\n");
            auto propChangedArgs = dynamic_cast<Manager::MemberPropertyChangedEventArgs^>(item->EventArgs);
            auto prop = JsonObject::Parse(propChangedArgs->Properties);

            GetChatIntegrationLayer()->ChangeChannelForUser(
                propChangedArgs->Member->XboxUserId->Data(),
                static_cast<uint8>(prop->GetNamedNumber(L"channel"))
                );

            break;
        }

        case MultiplayerEventType::PerformQosMeasurements:
            DebugTrace("MultiplayerEventType::PerformQosMeasurements\n");
            break;

        case MultiplayerEventType::SessionPropertyChanged:
            DebugTrace("MultiplayerEventType::SessionPropertyChanged\n");
            break;

        case MultiplayerEventType::SessionPropertyWriteCompleted:
            DebugTrace("MultiplayerEventType::SessionPropertyWriteCompleted\n");
            break;

        case MultiplayerEventType::SessionSynchronizedPropertyWriteCompleted:
            DebugTrace("MultiplayerEventType::SessionSynchronizedPropertyWriteCompleted\n");
            break;

        case MultiplayerEventType::SynchronizedHostWriteCompleted:
            DebugTrace("MultiplayerEventType::SynchronizedHostWriteCompleted\n");
            break;

        case MultiplayerEventType::UserAdded:
        {
            DebugTrace("MultiplayerEventType::UserAdded\n");

            auto userAddedArgs = dynamic_cast<Manager::UserAddedEventArgs^>(item->EventArgs);

            for (auto member : m_multiplayerManager->LobbySession->Members)
            {
                if (member->XboxUserId->Equals(userAddedArgs->XboxUserId) && member->IsLocal)
                {
                    if (m_meshManager == nullptr)
                    {
                        InitializeMeshManager(static_cast<BYTE>(member->MemberId));
                    }

                    SetupChat();
                }
            }

            break;
        }

        case MultiplayerEventType::UserRemoved:
        {
            DebugTrace("MultiplayerEventType::UserRemoved\n");

            auto userRemovedArgs = dynamic_cast<Manager::UserRemovedEventArgs^>(item->EventArgs);

            if (userRemovedArgs != nullptr)
            {
                if (m_inChat)
                {
                    LeaveChat();
                }

                auto users = User::Users;
                for (auto user : users)
                {
                    if (user->XboxUserId->Equals(userRemovedArgs->XboxUserId))
                    {
                        LeaveSession();
                        break;
                    }
                }
            }

            m_leavingToJoin = false;

            break;
        }

        default:
            DebugTrace("Unkonwn MultiplayerEventType encountered: %d\n", static_cast<int>(item->EventType));
            break;
        }
    }

    GetChatIntegrationLayer()->ProcessDataFrames();
}

#pragma region NetworkHandling
void Sample::InitializeMeshManager(uint8_t consoleID)
{
    DebugTrace("Sample::InitializeMeshManager(%d)\n", consoleID);

    if (m_meshInitialized)
    {
        return;
    }

    // Initialize winsock and the network layer
    try
    {
        m_meshManager = ref new Microsoft::Xbox::Samples::NetworkMesh::MeshManager(
            consoleID,
            SECURE_DEVICE_ASSOCIATION_TEMPLATE_UDP,
            Utils::GetDebugNameForLocalConsole(),
            false  // drop out-of-order packets
        );

        // Register event handlers that are fired when various network packets arrive
        RegisterMeshControllerEventHandlers();

        if (m_multiplayerManager != nullptr &&
            m_multiplayerManager->LobbySession != nullptr)
        {
            // Connect to peers
            for (auto member : m_multiplayerManager->LobbySession->Members)
            {
                if (member->IsLocal || member->ConnectionAddress->IsEmpty())
                {
                    continue;
                }

                m_meshManager->ConnectToAddress(
                    Windows::Xbox::Networking::SecureDeviceAddress::FromBase64String(member->ConnectionAddress),
                    member->DebugGamertag
                );
            }
        }

        m_meshInitialized = true;
    }
    catch (Platform::Exception^ ex)
    {
        DebugTrace("Exception creating mesh: 0x%x\n", ex->HResult);
    }
}

void Sample::ShutdownMeshManager()
{
    DebugTrace("Sample::ShutdownMeshManager()\n");

    if (m_meshManager != nullptr)
    {
        m_meshManager->GetMeshPacketManager()->OnChatMessageReceived -= m_chatToken;
        m_meshManager->OnDebugMessage -= m_debugToken;
        m_meshManager->OnPostHandshake -= m_handshakeToken;
        m_meshManager->OnDisconnected -= m_disconnectToken;

        m_meshManager->Shutdown();
        m_meshManager = nullptr;

        m_meshInitialized = false;
    }
}

void Sample::RegisterMeshControllerEventHandlers()
{
    DebugTrace("Sample::RegisterMeshControllerEventHandlers()\n");

    m_debugToken = m_meshManager->OnDebugMessage += ref new EventHandler<Microsoft::Xbox::Samples::NetworkMesh::DebugMessageEventArgs^>(
        [this](Platform::Object^, Microsoft::Xbox::Samples::NetworkMesh::DebugMessageEventArgs^ args)
    {
        DebugTrace("MeshManager::OnDebugMessage()\n");
        LogToConsole(args->Message);
    });

    m_handshakeToken = m_meshManager->OnPostHandshake += ref new EventHandler<MeshConnection^>(
        [this](Platform::Object^, MeshConnection^ args)
    {
        DebugTrace("MeshManager::OnPostHandshake()\n");
        if (m_multiplayerManager)
        {
            std::vector<std::wstring> xuids;

            auto members = m_multiplayerManager->LobbySession->Members;

            for (auto member : members)
            {
                if (!member->IsLocal && member->ConnectionAddress == args->GetSecureDeviceAddress()->GetBase64String())
                {
                    xuids.push_back(member->XboxUserId->Data());
                }
            }

            GetChatIntegrationLayer()->OnSecureDeviceAssocationConnectionEstablished(args, xuids);
        }
        else
        {
            DebugTrace("MultiplayerManager is null\n");
        }
    });

    m_disconnectToken = m_meshManager->OnDisconnected += ref new EventHandler<MeshConnection^>(
        [this](Platform::Object^, MeshConnection^ args)
    {
        DebugTrace("MeshManager::OnDisconnected()\n");
        GetChatIntegrationLayer()->RemoveRemoteConsole(args->GetRemoteId());
    });

    m_chatToken = m_meshManager->GetMeshPacketManager()->OnChatMessageReceived += ref new EventHandler<MeshChatMessageReceivedEvent^>(
        [this](Platform::Object^, MeshChatMessageReceivedEvent^ args)
    {
        Windows::Storage::Streams::IBuffer^ chatPacket = args->Buffer;
        GetChatIntegrationLayer()->OnIncomingChatMessage(chatPacket, args->Sender->GetRemoteId());
    });
}
#pragma endregion
