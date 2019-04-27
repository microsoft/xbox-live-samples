//--------------------------------------------------------------------------------------
// GameBoardScreen.cpp
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "GameBoardScreen.h"

#include "ConfirmPopUpScreen.h"
#include "GameSaveManager.h"
#include "SampleGame.h"

using namespace DirectX;

using namespace winrt::Windows::Xbox::Input;

namespace GameSaveSample {

namespace
{
#pragma region Game Board Draw Constants
    const auto c_saveSlotLeftControlHelp_Center = DirectX::XMFLOAT2(314.f, 118.f);
    const auto c_saveSlotRightControlHelp_Center = DirectX::XMFLOAT2(854.f, 118.f);
    const auto c_saveSlotFirstNumber_TopCenter = DirectX::XMFLOAT2(399.f, 89.f);
    const auto c_saveSlotNumber_PixelsBetweenCenters = 196.f;
    const auto c_saveSlotNumber_Radius = 23.f;
    const auto c_saveSlotSecondNumber_TopCenter = DirectX::XMFLOAT2(595.f, 89.f);
    const auto c_saveSlotThirdNumber_TopCenter = DirectX::XMFLOAT2(781.f, 89.f);
    const auto c_saveSlotFirstNumber_CircleCenter = DirectX::XMFLOAT2(399.f, 116.f);
    const auto c_saveSlotSecondNumber_CircleCenter = DirectX::XMFLOAT2(595.f, 116.f);
    const auto c_saveSlotThirdNumber_CircleCenter = DirectX::XMFLOAT2(781.f, 116.f);

    const auto c_gameTitle_UpperLeft = DirectX::XMFLOAT2(190.f, 170.f);

    const auto c_playerName_UpperLeft = DirectX::XMFLOAT2(200.f, 245.f);
    const auto c_playerName_Scale = 0.85f;

    const auto c_gameSaveMetadata_UpperLeft = DirectX::XMFLOAT2(200.f, 270.f);
    const auto c_gameSaveMetadata_Scale = 0.85f;
    const auto c_gameSaveAdditionalMetadata_UpperLeft = DirectX::XMFLOAT2(200.f, 295.f);

    const auto c_currentScore_UpperLeft = DirectX::XMFLOAT2(100.f, 375.f);
    const auto c_currentScore_Scale = 1.75f;
    const auto c_currentScoreLabel_UpperLeft = DirectX::XMFLOAT2(110.f, 440.f);

    const DirectX::XMVECTORF32 c_lettersRemaining_Color = { 230.f / 255.f, 178.f / 255.f, 138.f / 255.f, 1.f };
    const auto c_lettersRemaining_UpperLeft = DirectX::XMFLOAT2(255.f, 375.f);
    const auto c_lettersRemaining_LetterOffset = DirectX::XMFLOAT2(75.f, 23.f);
    const auto c_lettersRemaining_Scale = 0.8f;
    const auto c_lettersRemainingHelp_Center = DirectX::XMFLOAT2(600.f, 460.f);

    const auto c_letterTileRegion = RECT{ 160 - 42, 530 - 42, 538 + 42, 918 + 42 };
    const auto c_letterTileFirstTile_Center = DirectX::XMFLOAT2(160.f, 530.f);
    const auto c_letterTileRadius = 42.f;
    const auto c_letterTile_CenterOffset = DirectX::XMFLOAT2(94.f, 97.f);
    const auto c_letterTileValue_OffsetFromTileCenter = DirectX::XMFLOAT2(16.f, 8.f);
    const auto c_letterTileScrollDelay_Seconds = 0.2f;

    const auto c_wordLinkFirstHorizontal_Center = DirectX::XMFLOAT2(210.f, 530.f);
    const auto c_wordLinkFirstVertical_Center = DirectX::XMFLOAT2(160.f, 582.f);

    const winrt::Windows::Foundation::Rect c_menu_Region = { 635.f, 490.f, 500.f, 500.f };

    const auto c_inputControlsHelp_UpperLeft = DirectX::XMFLOAT2(100.f, 1010.f);
    const auto c_inputControlsHelp_Scale = 0.6f;

    const winrt::Windows::Foundation::Rect c_debugLog_Region = { 1240.f, 150.f, 620.f, 800.f };
    const auto c_debugLogScrollUpIcon_Center = DirectX::XMFLOAT2(1560.f, 130.f);
    const auto c_debugLogScrollDownIcon_Center = DirectX::XMFLOAT2(1560.f, 970.f);
    const auto c_debugLogScrollDelay_Seconds = 0.1f;

    // Letter count
    const int c_letterCounts[] =
    {
        5, // A
        3, // B
        3, // C
        3, // D
        6, // E
        3, // F
        3, // G
        3, // H
        4, // I
        1, // J
        2, // K
        4, // L
        3, // M
        4, // N
        5, // O
        3, // P
        1, // Q
        4, // R
        4, // S
        6, // T
        3, // U
        2, // V
        2, // W
        1, // X
        3, // Y
        2 // Z
    };

    // Letter values
    const int c_letterValues[] =
    {
        1, // A
        3, // B
        3, // C
        2, // D
        1, // E
        4, // F
        2, // G
        4, // H
        1, // I
        8, // J
        5, // K
        1, // L
        3, // M
        1, // N
        1, // O
        3, // P
        10, // Q
        1, // R
        1, // S
        1, // T
        1, // U
        4, // V
        4, // W
        8, // X
        4, // Y
        10 // Z
    };
#pragma endregion
}

GameBoardScreen::GameBoardScreen(const std::shared_ptr<ScreenManager>& screenManager) : 
    MenuScreen(screenManager),
    m_cursorPosition(XMUINT2(0, 0)),
    m_firstSlotToDisplay(1),
    m_isGameSaveManagerInitialized(false),
    m_isOverLimitOnLetters(false),
    m_lettersRemaining(c_letterCounts, c_letterCounts + sizeof(c_letterCounts) / sizeof(c_letterCounts[0])),
    m_logFontHeight(0),
    m_logLineBegin(0),
    m_logScrollDownDelay(0),
    m_logScrollUpDelay(0),
    m_score(0),
    m_tileScrollDownDelay(0),
    m_tileScrollUpDelay(0),
    m_wordListLoaded(false)
{
    // Setup word tracker
    auto boardTiles = c_boardWidth * c_boardHeight;
    for (uint32_t i = 1; i <= boardTiles; ++i)
    {
        m_wordTracker.push_back(WordTrackerTile());
    }

    // Setup menu display parameters
    m_animateSelected = false;
    m_menuActive = false;
    m_menuSpacing = 1.4f;
    m_showCurrentUser = false;
    SetCenterJustified(false);

    // Setup menu entries
    ClearMenuEntries();

	m_menuEntries.push_back(MenuEntry(L"Get Board", [this](int player) -> winrt::Windows::Foundation::IAsyncAction
	{
		// make sure log auto-scroll is on so that sample user sees the results from this action
		m_logLineBegin = 0;

		// load or reload game state using GetAsync()
		auto activeBoardMetadata = Game->GameSaveManager.ActiveBoardGameSave()->m_containerMetadata;
		auto activeBoardContainerName = activeBoardMetadata->m_containerName;
		auto activeBoardNeedsSync = activeBoardMetadata->m_needsSync;
		co_await Game->GameSaveManager.Get();
		{
			if (activeBoardNeedsSync)
			{
				Game->GameSaveManager.LoadContainerMetadata(activeBoardContainerName);
			}
		}
	}));

	m_menuEntries.push_back(MenuEntry(L"Read Board", [this](int player) -> winrt::Windows::Foundation::IAsyncAction
	{
		// make sure log auto-scroll is on so that sample user sees the results from this action
		m_logLineBegin = 0;

		// load or reload game state using ReadAsync()
		auto activeBoardMetadata = Game->GameSaveManager.ActiveBoardGameSave()->m_containerMetadata;
		auto activeBoardContainerName = activeBoardMetadata->m_containerName;
		auto activeBoardNeedsSync = activeBoardMetadata->m_needsSync;
		co_await Game->GameSaveManager.Read();
		{
			if (activeBoardNeedsSync)
			{
				Game->GameSaveManager.LoadContainerMetadata(activeBoardContainerName);
			}
		}
	}));

	m_menuEntries.push_back(MenuEntry(L"Save Board", [this](int player) -> winrt::Windows::Foundation::IAsyncAction
	{
		// make sure log auto-scroll is on so that sample user sees the results from this action
		m_logLineBegin = 0;

		// save game state
		auto activeBoardContainerName = Game->GameSaveManager.ActiveBoardGameSave()->m_containerMetadata->m_containerName;

		co_await Game->GameSaveManager.Save(false);

		// update the index while we're saving this board, if it's dirty
		co_await Game->GameSaveManager.SaveIndex(true);
		co_await Game->GameSaveManager.LoadContainerMetadata(activeBoardContainerName);
		co_await Game->GameSaveManager.GetRemainingQuota();
	}));

	m_menuEntries.push_back(MenuEntry(L"Reset Board", [this](int) -> winrt::Windows::Foundation::IAsyncAction
	{
		// make sure log auto-scroll is on so that sample user sees the results from this action
		m_logLineBegin = 0;

		// clear board of letter tiles, marking it dirty if appropriate
		std::lock_guard<std::mutex> lock(Game->GameSaveManager.ActiveBoardGameSave()->m_mutex);

		auto& gameBoard = Game->GameSaveManager.ActiveBoard();
		gameBoard.ResetBoard();

		Log::WriteAndDisplay(L"Game board %d reset\n", Game->GameSaveManager.ActiveBoardNumber());

		auto activeGameSave = Game->GameSaveManager.ActiveBoardGameSave();
		// (being explicit here for readability...)
		if (activeGameSave->m_isGameDataLoaded)
		{
			activeGameSave->m_isGameDataDirty = true;
		}
		else
		{
			activeGameSave->m_isGameDataDirty = false;
		}

		return winrt::Windows::Foundation::IAsyncAction(nullptr);
	}));

	m_menuEntries.push_back(MenuEntry(L"Delete Board", [this](int player) -> winrt::Windows::Foundation::IAsyncAction
	{
		// make sure log auto-scroll is on so that sample user sees the results from this action
		m_logLineBegin = 0;

		// delete game state (resets board)
		auto activeBoardContainerName = Game->GameSaveManager.ActiveBoardGameSave()->m_containerMetadata->m_containerName;
		co_await Game->GameSaveManager.Delete();

		co_await Game->GameSaveManager.LoadContainerMetadata(activeBoardContainerName);

		co_await Game->GameSaveManager.GetRemainingQuota();
	}));

	m_menuEntries.push_back(MenuEntry(L"Delete Board Blob", [this](int player) -> winrt::Windows::Foundation::IAsyncAction
	{
		// make sure log auto-scroll is on so that sample user sees the results from this action
		m_logLineBegin = 0;

		// delete game state blobs (but not container) (resets board)
		auto activeBoardContainerName = Game->GameSaveManager.ActiveBoardGameSave()->m_containerMetadata->m_containerName;
		co_await Game->GameSaveManager.DeleteBlobs();

		co_await Game->GameSaveManager.LoadContainerMetadata(activeBoardContainerName);

		co_await Game->GameSaveManager.GetRemainingQuota();
	}));

	m_menuEntries.push_back(MenuEntry(L"List Containers", [this](int player) -> winrt::Windows::Foundation::IAsyncAction
	{
		// make sure log auto-scroll is on so that sample user sees the results from this action
		m_logLineBegin = 0;

		// start a container query
		Log::WriteAndDisplay(L"Container query started...\n");

		co_await Game->GameSaveManager.LoadContainerMetadata(L"", false);

		Log::WriteAndDisplay(L"Container query complete\n");

		Game->GameSaveManager.WriteGameSaveMetadataToDisplayLog(false);
	}));

	m_menuEntries.push_back(MenuEntry(L"List Containers & Blobs", [this](int player) -> winrt::Windows::Foundation::IAsyncAction
	{
		// make sure log auto-scroll is on so that sample user sees the results from this action
		m_logLineBegin = 0;

		// start a container AND blob query
		Log::WriteAndDisplay(L"Container and blob query started...\n");

		co_await Game->GameSaveManager.LoadContainerMetadata(L"", true);

		Log::WriteAndDisplay(L"Container and blob query complete\n");

		Game->GameSaveManager.WriteGameSaveMetadataToDisplayLog(true);
	}));
}

GameBoardScreen::~GameBoardScreen()
{
}

void GameBoardScreen::LoadContent()
{
    MenuScreen::LoadContent();

    auto contentManager = Manager()->GetContentManager();

    // Load board textures
    m_background = contentManager->LoadTexture(L"Assets\\Board\\BOARD_BG.jpg");
    m_cursor = contentManager->LoadTexture(L"Assets\\Board\\gameboard_letter_ON.png");
    m_hortizontalWordLinker = contentManager->LoadTexture(L"Assets\\Board\\wordformed_arrow_horiz.png");
    m_verticalWordLinker = contentManager->LoadTexture(L"Assets\\Board\\wordformed_arrow_vert.png");

    for (auto i = 1; i <= c_saveSlotCount; ++i)
    {
        std::wstring saveSlotAssetPath(L"Assets\\Buttons\\btnSaveSlot0");
        saveSlotAssetPath += std::to_wstring(i);

        std::wstring saveSlotOnAssetPath = saveSlotAssetPath;

        saveSlotAssetPath.append(L".png");
        saveSlotOnAssetPath.append(L"_ON.png");

        m_saveSlotNumbers.push_back(contentManager->LoadTexture(saveSlotAssetPath));
        m_saveSlotActiveNumbers.push_back(contentManager->LoadTexture(saveSlotOnAssetPath));
    }

    // Load input controls textures
    m_clearLetterControl = contentManager->LoadTexture(L"Assets\\Controller\\x.png");
    m_logScrollDownControl = contentManager->LoadTexture(L"Assets\\Controller\\rs.png");
    m_logScrollUpControl = contentManager->LoadTexture(L"Assets\\Controller\\rs.png");
    m_menuControl = contentManager->LoadTexture(L"Assets\\Controller\\dpad.png");
    m_moveCursorControl = contentManager->LoadTexture(L"Assets\\Controller\\ls.png");
    m_saveSlotLeftControl = contentManager->LoadTexture(L"Assets\\Controller\\lb.png");
    m_saveSlotRightControl = contentManager->LoadTexture(L"Assets\\Controller\\rb.png");
    m_swapLetterControl = contentManager->LoadTexture(L"Assets\\Controller\\rs.png");

    // Load fonts
    auto device = Manager()->GetDevice();
    m_gameBoardControlsHelpFont = std::make_shared<SpriteFont>(device, L"Assets\\Fonts\\SegoeUI_24_NP.spritefont");
    m_gameBoardLettersRemainingFont = std::make_shared<SpriteFont>(device, L"Assets\\Fonts\\Consolas_16_NP.spritefont");
    m_gameBoardMetadataFont = std::make_shared<SpriteFont>(device, L"Assets\\Fonts\\SegoeUISemilight_18_NP.spritefont");
    m_gameBoardTitleFont = std::make_shared<SpriteFont>(device, L"Assets\\Fonts\\SegoeUILight_42_NP.spritefont");
    m_gameBoardTileValueFont = std::make_shared<SpriteFont>(device, L"Assets\\Fonts\\SegoeUI_24_NP.spritefont");
    m_gameBoardTileFont = std::make_shared<SpriteFont>(device, L"Assets\\Fonts\\SegoeUISemilight_42_NP.spritefont");
    m_logFont = Manager()->GetDebugFont();
    XMVECTOR logFontMeasurement = m_logFont->MeasureString(L"X");
    m_logFontHeight = XMVectorGetY(logFontMeasurement);

    // Load word list
    Log::WriteAndDisplay(L"Loading word list...\n");
	LoadWordlistAsync(contentManager);

	m_gameBoardMetadataFont->SetDefaultCharacter(L'*');
}

winrt::Windows::Foundation::IAsyncAction GameBoardScreen::LoadWordlistAsync(std::shared_ptr<GameSaveSample::ContentManager> & contentManager)
{
	auto wordList = co_await contentManager->LoadWordList(L"Assets\\TWL06_2to5.txt");
	{
		m_wordList = wordList;
		m_wordListLoaded = true;
		Log::WriteAndDisplay(L"Word list loaded\n");
	}
}

void GameBoardScreen::Update(float totalTime, float elapsedTime, bool otherScreenHasFocus, bool coveredByOtherScreen)
{
    MenuScreen::Update(totalTime, elapsedTime, otherScreenHasFocus, coveredByOtherScreen);
    
    if (m_logScrollDownDelay > 0)
    {
        m_logScrollDownDelay -= elapsedTime;
    }
    if (m_logScrollUpDelay > 0)
    {
        m_logScrollUpDelay -= elapsedTime;
    }

    if (m_tileScrollDownDelay > 0)
    {
        m_tileScrollDownDelay -= elapsedTime;
    }
    if (m_tileScrollUpDelay > 0)
    {
        m_tileScrollUpDelay -= elapsedTime;
    }

    if (!m_isGameSaveManagerInitialized)
    {
        // Has the manager initialized since last checked?
        if (Game->GameSaveManager.IsInitialized())
        {
            // Perform save slot display setup
            auto activeBoardNum = Game->GameSaveManager.ActiveBoardNumber();
            if (activeBoardNum <= 3)
            {
                m_firstSlotToDisplay = 1;
            }
            else
            {
                m_firstSlotToDisplay = std::min(activeBoardNum, c_saveSlotCount - 2);
            }
        }
    }
    m_isGameSaveManagerInitialized = Game->GameSaveManager.IsInitialized();

    if (Game->GameSaveManager.HasActiveBoard())
    {
        std::lock_guard<std::mutex> lock(Game->GameSaveManager.ActiveBoardGameSave()->m_mutex);

        UpdateLettersRemaining();

        if (m_wordListLoaded)
        {
            UpdateScore();
        }
    }
}

void GameBoardScreen::HandleInput(const DirectX::InputState & input)
{
    // Handle menu input
    MenuScreen::HandleInput(input);

    // Handle non-menu game board input if there's an active board
    GameSaveManager &gameSaveManager = Game->GameSaveManager;

    if (gameSaveManager.HasActiveBoard())
    {
        std::lock_guard<std::mutex> lock(gameSaveManager.ActiveBoardGameSave()->m_mutex);

        int controllingPlayer = GetControllingPlayer();
        auto activeBoard = gameSaveManager.ActiveBoard();
        auto activeBoardNum = gameSaveManager.ActiveBoardNumber();
        auto& gameBoard = gameSaveManager.ActiveBoard();
        auto& currentTile = gameBoard.GetGameTile(m_cursorPosition);
        auto letterPressed = input.GetLetterPressed();
        auto numberPressed = input.GetNumberPressed();
        XMINT2 mouseClick = XMINT2();

        if (!m_menuActive && letterPressed > 0)
        {
            if (!currentTile.m_placed || (currentTile.m_letter != letterPressed))
            {
                currentTile.m_placed = true;
                currentTile.m_letter = letterPressed;
                gameSaveManager.MarkActiveBoardDirty();
            }
        }
        else if (numberPressed > 0 && numberPressed <= c_saveSlotCount)
        {
            if (numberPressed < activeBoardNum)
            {
                gameSaveManager.ActiveBoardNumber(numberPressed);
                m_firstSlotToDisplay = std::min(numberPressed, m_firstSlotToDisplay);
            }
            else if (numberPressed > activeBoardNum)
            {
                gameSaveManager.ActiveBoardNumber(numberPressed);
                m_firstSlotToDisplay = std::max(m_firstSlotToDisplay, numberPressed - 2);
            }
        }
        else if (input.IsMouseSelect(mouseClick))
        {
            auto localMouseClick = DX::ConvertWindowPixelToLocalCoord(Manager()->GetWindowBounds(), mouseClick);
            auto localMouseClickF = XMFLOAT2(float(localMouseClick.x), float(localMouseClick.y));
            if (DX::IsPointInsideRectangle(localMouseClick, c_letterTileRegion))
            {
                for (uint32_t j = 0; j < activeBoard.m_boardHeight; ++j)
                {
                    for (uint32_t i = 0; i < activeBoard.m_boardWidth; ++i)
                    {
                        XMFLOAT2 tileCenter = XMFLOAT2(c_letterTileFirstTile_Center.x + (c_letterTile_CenterOffset.x * i), c_letterTileFirstTile_Center.y + (c_letterTile_CenterOffset.y * j));
                        if (DX::IsPointInsideCircle(localMouseClickF, tileCenter, c_letterTileRadius))
                        {
                            m_cursorPosition.x = i;
                            m_cursorPosition.y = j;
                            m_menuActive = false;
                            return;
                        }
                    }
                }
            }
            else if (DX::IsPointInsideCircle(localMouseClickF, c_saveSlotFirstNumber_CircleCenter, c_saveSlotNumber_Radius))
            {
                gameSaveManager.ActiveBoardNumber(m_firstSlotToDisplay);
            }
            else if (DX::IsPointInsideCircle(localMouseClickF, c_saveSlotSecondNumber_CircleCenter, c_saveSlotNumber_Radius))
            {
                gameSaveManager.ActiveBoardNumber(m_firstSlotToDisplay + 1);
            }
            else if (DX::IsPointInsideCircle(localMouseClickF, c_saveSlotThirdNumber_CircleCenter, c_saveSlotNumber_Radius))
            {
                gameSaveManager.ActiveBoardNumber(m_firstSlotToDisplay + 2);
            }
        }
        else if (!m_menuActive &&
            (input.IsNewButtonPress(GamepadButtons::X, controllingPlayer)
            || input.IsNewKeyPress(Keyboard::Keys::Space)
            || input.IsNewKeyPress(Keyboard::Keys::Delete)))
        {
            if (currentTile.m_placed)
            {
                gameSaveManager.MarkActiveBoardDirty();
            }
            currentTile.m_letter = 0;
            currentTile.m_placed = false;
        }
        else if (!m_menuActive && input.IsTileScrollUp(controllingPlayer))
        {
            if (m_tileScrollUpDelay <= 0)
            {
                // advance tile value
                if (!currentTile.m_placed)
                {
                    currentTile.m_placed = true;
                    currentTile.m_letter = L'A';
                }
                else
                {
                    if (currentTile.m_letter >= L'A'
                        && currentTile.m_letter < L'Z')
                    {
                        currentTile.m_letter++;
                    }
                    else
                    {
                        currentTile.m_letter = L'A';
                    }
                }

                gameSaveManager.MarkActiveBoardDirty();
                m_tileScrollUpDelay = c_letterTileScrollDelay_Seconds;
            }
        }
        else if (!m_menuActive && input.IsTileScrollDown(controllingPlayer))
        {
            if (m_tileScrollDownDelay <= 0)
            {
                // decrease tile value
                if (!currentTile.m_placed)
                {
                    currentTile.m_placed = true;
                    currentTile.m_letter = L'Z';
                }
                else
                {
                    if (currentTile.m_letter > L'A'
                        && currentTile.m_letter <= L'Z')
                    {
                        currentTile.m_letter--;
                    }
                    else
                    {
                        currentTile.m_letter = L'Z';
                    }
                }

                gameSaveManager.MarkActiveBoardDirty();
                m_tileScrollDownDelay = c_letterTileScrollDelay_Seconds;
            }
        }
        else if (input.IsLogScrollUp())
        {
            size_t maxDisplayLines = static_cast<size_t>(c_debugLog_Region.Height / m_logFontHeight);
            auto logIndexSize = Log::g_displayLog.size();
            if (m_logLineBegin == 0)
            {
                if (logIndexSize > maxDisplayLines)
                {
                    // stop auto-scroll at current top line
                    m_logLineBegin = logIndexSize - maxDisplayLines;
                }
            }
            else
            {
                if (m_logScrollUpDelay <= 0)
                {
                    // scroll up
                    m_logLineBegin = std::max(m_logLineBegin - 1, size_t(1));
                    m_logScrollUpDelay = c_debugLogScrollDelay_Seconds;
                }
            }
        }
        else if (input.IsLogScrollDown())
        {
            if (m_logLineBegin > 0)
            {
                size_t maxDisplayLines = static_cast<size_t>(c_debugLog_Region.Height / m_logFontHeight);
                auto logIndexSize = Log::g_displayLog.size();

                if ((m_logLineBegin + 1) > (logIndexSize - maxDisplayLines))
                {
                    // turn auto-scroll back on
                    m_logLineBegin = 0;
                }

                if (m_logScrollDownDelay <= 0)
                {
                    // scroll down
                    m_logLineBegin++;
                    m_logScrollDownDelay = c_debugLogScrollDelay_Seconds;
                }
            }
        }
        else if (input.IsNewKeyPress(Keyboard::Keys::Home))
        {
            size_t maxDisplayLines = static_cast<size_t>(c_debugLog_Region.Height / m_logFontHeight);
            auto logIndexSize = Log::g_displayLog.size();
            if (logIndexSize > maxDisplayLines)
            {
                m_logLineBegin = 1;
            }
            else
            {
                m_logLineBegin = 0;
            }
        }
        else if (input.IsNewKeyPress(Keyboard::Keys::End))
        {
            m_logLineBegin = 0;
        }
        else if (input.IsNewButtonPress(GamepadButtons::LeftShoulder, controllingPlayer) && activeBoardNum > 1)
        {
            auto newActiveBoardNum = activeBoardNum - 1;
            gameSaveManager.ActiveBoardNumber(newActiveBoardNum);
            m_firstSlotToDisplay = std::min(newActiveBoardNum, m_firstSlotToDisplay);
        }
        else if (input.IsNewButtonPress(GamepadButtons::RightShoulder, controllingPlayer) && activeBoardNum < c_saveSlotCount)
        {
            auto newActiveBoardNum = activeBoardNum + 1;
            gameSaveManager.ActiveBoardNumber(newActiveBoardNum);
            m_firstSlotToDisplay = std::max(m_firstSlotToDisplay, newActiveBoardNum - 2);
        }
        else if (input.IsCursorLeft(controllingPlayer, nullptr))
        {
            if (m_menuActive)
            {
                m_menuActive = false;
                m_cursorPosition.x = gameBoard.m_boardWidth - 1;
            }
            else if (m_cursorPosition.x > 0)
            {
                m_cursorPosition.x--;
            }
            else
            {
                m_menuActive = true;
            }
        }
        else if (input.IsCursorRight(controllingPlayer, nullptr))
        {
            if (m_menuActive)
            {
                m_menuActive = false;
                m_cursorPosition.x = 0;
            }
            else if (m_cursorPosition.x < gameBoard.m_boardWidth - 1)
            {
                m_cursorPosition.x++;
            }
            else
            {
                m_menuActive = true;
            }

        }
        else if (!m_menuActive && input.IsCursorUp(controllingPlayer, nullptr))
        {
            if (m_cursorPosition.y > 0)
            {
                m_cursorPosition.y--;
            }
            else
            {
                m_cursorPosition.y = gameBoard.m_boardHeight - 1;
            }
        }
        else if (!m_menuActive && input.IsCursorDown(controllingPlayer, nullptr))
        {
            if (m_cursorPosition.y < gameBoard.m_boardHeight - 1)
            {
                m_cursorPosition.y++;
            }
            else
            {
                m_cursorPosition.y = 0;
            }
        }
    } // HasActiveBoard
}

void GameBoardScreen::Draw(float totalTime, float elapsedTime)
{
    auto spriteBatch = Manager()->GetSpriteBatch();
    auto gameFont = Manager()->GetSpriteFont();
    auto blendStates = Manager()->GetCommonStates();
    auto scaleMatrix = DX::GetScaleMatrixForWindow(Manager()->GetWindowBounds());

    // Draw background
    spriteBatch->Begin(SpriteSortMode_Deferred, blendStates->NonPremultiplied(), nullptr, nullptr, nullptr, nullptr, scaleMatrix);
    spriteBatch->Draw(m_background->GetResourceViewTemporary(), XMFLOAT2(0, 0));
    spriteBatch->End();

    if (IsActive() && m_isGameSaveManagerInitialized)
    {
        MenuScreen::Draw(totalTime, elapsedTime);
    }

    if (m_state != ScreenState::Active)
    {
        return;
    }

    GameSaveManager &gameSaveManager = Game->GameSaveManager;
    assert(gameSaveManager.HasActiveBoard());

    std::lock_guard<std::mutex> lock(gameSaveManager.ActiveBoardGameSave()->m_mutex);

    auto activeBoardNum = gameSaveManager.ActiveBoardNumber();
    auto activeBoard = gameSaveManager.ActiveBoard();
    auto activeBoardGameSave = gameSaveManager.ActiveBoardGameSave();
    auto activeBoardMetadata = activeBoardGameSave->m_containerMetadata;

    spriteBatch->Begin(SpriteSortMode_Deferred, blendStates->NonPremultiplied(), nullptr, nullptr, nullptr, nullptr, scaleMatrix);

    // Draw numbers of the visible game boards and highlight the active one
    XMFLOAT2 position = c_saveSlotFirstNumber_TopCenter;
    XMFLOAT2 origin = XMFLOAT2(m_saveSlotNumbers[0]->Width() / 2.0f, 0.0f);

    for (auto i = m_firstSlotToDisplay; i < m_firstSlotToDisplay + 3; ++i)
    {
        if (i == m_firstSlotToDisplay + 2)
            position.x -= 10; // UI correction: 3rd board number is not the same distance from the 2nd as the 2nd is from the 1st

        if (i == activeBoardNum)
        {
            spriteBatch->Draw(m_saveSlotActiveNumbers[i - 1]->GetResourceViewTemporary(), position, nullptr, Colors::White, 0.0f, origin);
        }
        else
        {
            spriteBatch->Draw(m_saveSlotNumbers[i - 1]->GetResourceViewTemporary(), position, nullptr, Colors::White, 0.0f, origin);
        }

        position.x += c_saveSlotNumber_PixelsBetweenCenters;
    }

    // Draw active board title and dirty status
	wchar_t gameBoardTitleStr[100];
    if (activeBoardNum > 0)
    {
		swprintf_s(gameBoardTitleStr, L"WordGame Board %i", activeBoardNum);
    }
    if (gameSaveManager.IsActiveBoardDirty())
    {
		swprintf_s(gameBoardTitleStr, L"WordGame Board*");
    }
    m_gameBoardTitleFont->DrawString(spriteBatch.get(), gameBoardTitleStr, c_gameTitle_UpperLeft);

    // Draw player name
	wchar_t userDisplayStr[100];
	swprintf_s(userDisplayStr, L"User: %s", Game->LiveResources->GetGamertag().c_str());
    m_gameBoardMetadataFont->DrawString(spriteBatch.get(), userDisplayStr, c_playerName_UpperLeft, Colors::White, 0.0f, XMFLOAT2(0, 0), c_playerName_Scale);

    // Draw active board metadata (last save date, current user gamertag)
    std::wstring gameBoardMetadataDisplay;
    if (activeBoardMetadata->m_isGameDataOnDisk)
    {
        gameBoardMetadataDisplay.append(activeBoardGameSave->m_isGameDataLoaded ? L"BOARD LOADED" : L"BOARD NOT LOADED");

        if (activeBoardMetadata->m_needsSync)
        {
            gameBoardMetadataDisplay.append(L"  (NEEDS SYNC)");
        }

		gameBoardMetadataDisplay.append(L"  (Last Modified: ");
		gameBoardMetadataDisplay.append(FormatLocalTimeFromDateTime(activeBoardMetadata->m_lastModified));
		gameBoardMetadataDisplay.append(L")");
    }
    else
    {
        if (gameSaveManager.IsActiveBoardDirty())
        {
            gameBoardMetadataDisplay.append(L"BOARD NOT SAVED");
        }
        else
        {
            gameBoardMetadataDisplay.append(L"BOARD NOT STARTED");
        }
    }
    m_gameBoardMetadataFont->DrawString(spriteBatch.get(), gameBoardMetadataDisplay.c_str(), c_gameSaveMetadata_UpperLeft, Colors::White, 0.0f, XMFLOAT2(0, 0), c_gameSaveMetadata_Scale);

    // Draw active board dev metadata (sync status/mode)
    if (m_isGameSaveManagerInitialized)
    {
        std::wstring devModeMetadata = L"Sync Mode: ";
        if (gameSaveManager.IsSyncOnDemand())
        {
            devModeMetadata.append(L"Sync-On-Demand");
        }
        else
        {
            devModeMetadata.append(L"Full Sync");
        }
		devModeMetadata.append(L"     Remaining Quota: ");
		wchar_t remainingQuotaInBytes[20];
		_i64tow_s(gameSaveManager.RemainingQuotaInBytes(), remainingQuotaInBytes, _countof(remainingQuotaInBytes), 10);
		devModeMetadata.append(remainingQuotaInBytes);
		devModeMetadata.append(L" bytes");
        m_gameBoardMetadataFont->DrawString(spriteBatch.get(), devModeMetadata.c_str(), c_gameSaveAdditionalMetadata_UpperLeft, Colors::White, 0.0f, XMFLOAT2(0, 0), c_gameSaveMetadata_Scale);
    }
    else
    {
        m_gameBoardMetadataFont->DrawString(spriteBatch.get(), L"Sync Status: Not Initialized", c_gameSaveAdditionalMetadata_UpperLeft);
    }

    // Draw current score
    auto scoreColor = m_isOverLimitOnLetters ? Colors::Red : Colors::White;
	wchar_t scoreBuffer[20];
	_itow_s(m_score, scoreBuffer, 10);
    gameFont->DrawString(spriteBatch.get(), scoreBuffer, c_currentScore_UpperLeft, scoreColor, 0.0f, XMFLOAT2(0, 0), c_currentScore_Scale);
    m_gameBoardMetadataFont->DrawString(spriteBatch.get(), L"SCORE", c_currentScoreLabel_UpperLeft);

    // Draw remaining letters count
    if (m_lettersRemaining.size() >= 26)
    {
        XMFLOAT2 letterPosition = c_lettersRemaining_UpperLeft;
        for (wchar_t letter = L'A'; letter <= L'Z'; ++letter)
        {
            auto letterIndex = letter - L'A';
			auto lettersRemaining = m_lettersRemaining[letterIndex];
			wchar_t countStr[30];
			swprintf_s(countStr, L"%c=%i(%i)", letter, m_lettersRemaining[letterIndex], c_letterValues[letterIndex]);
            auto countColor = c_lettersRemaining_Color;
            if (lettersRemaining < 0)
            {
                countColor = Colors::Red;
            }
            else if (lettersRemaining == 0)
            {
                countColor = Colors::Yellow;
            }
            m_gameBoardLettersRemainingFont->DrawString(spriteBatch.get(), countStr, letterPosition, countColor, 0.0f, XMFLOAT2(0, 0), c_lettersRemaining_Scale);

            // Get next letter position
            if ((letterIndex + 1) % 9 == 0)
            {
                letterPosition.x = c_lettersRemaining_UpperLeft.x;
                letterPosition.y += c_lettersRemaining_LetterOffset.y;
            }
            else
            {
                letterPosition.x += c_lettersRemaining_LetterOffset.x;
            }
        }
    }

    // Draw help text for remaining letters section
    winrt::hstring lettersRemainingHelp = L"Letter = X pts (# remaining)";
    XMFLOAT2 lettersRemainingHelpPosition = c_lettersRemainingHelp_Center;
    auto lettersRemainingHelpSize = m_gameBoardLettersRemainingFont->MeasureString(lettersRemainingHelp.c_str());
    XMFLOAT2 lettersRemainingHelpOrigin = XMFLOAT2(XMVectorGetX(lettersRemainingHelpSize) / 2.0f, m_gameBoardLettersRemainingFont->GetLineSpacing() / 2.0f);
    m_gameBoardLettersRemainingFont->DrawString(spriteBatch.get(), lettersRemainingHelp.c_str(), c_lettersRemainingHelp_Center, c_lettersRemaining_Color, 0.0f, lettersRemainingHelpOrigin);

    // Draw tile cursor
    if (!m_menuActive)
    {
        XMFLOAT2 cursorCenter = XMFLOAT2(c_letterTileFirstTile_Center.x + (c_letterTile_CenterOffset.x * m_cursorPosition.x), c_letterTileFirstTile_Center.y + (c_letterTile_CenterOffset.y * m_cursorPosition.y));
        spriteBatch->Draw(m_cursor->GetResourceViewTemporary(), cursorCenter, nullptr, Colors::White, 0.0f, XMFLOAT2(float(m_cursor->Width() / 2), float(m_cursor->Height() / 2)));
    }

    // Draw placed letters
    for (uint32_t j = 0; j < activeBoard.m_boardHeight; ++j)
    {
        for (uint32_t i = 0; i < activeBoard.m_boardWidth; ++i)
        {
            XMFLOAT2 tileCenter = XMFLOAT2(c_letterTileFirstTile_Center.x + (c_letterTile_CenterOffset.x * i), c_letterTileFirstTile_Center.y + (c_letterTile_CenterOffset.y * j));
            auto currentTile = activeBoard.GetGameTile(XMUINT2(i, j));

            if (currentTile.m_placed)
            {
                if (currentTile.m_letter)
                {
                    // draw letter (centered on tile)
					wchar_t letter[2] = { currentTile.m_letter, L'\0' };
                    XMVECTOR letterSize = m_gameBoardTileFont->MeasureString(letter);
                    XMFLOAT2 letterOrigin = XMFLOAT2(XMVectorGetX(letterSize) / 2.0f + 6.0f, XMVectorGetY(letterSize) / 2.0f);
                    if (letter[0] == L'Q')
                    {
                        letterOrigin.x += 2.0f; // correct some display issues
                    }
                    m_gameBoardTileFont->DrawString(spriteBatch.get(), letter, tileCenter, Colors::Pink, 0.0f, letterOrigin, 0.75f);

                    // draw letter value (top-left-justified, offset from tile center)
					wchar_t letterValue[10];
					_itow_s(c_letterValues[letter[0] - L'A'], letterValue, 10);
                    XMFLOAT2 letterValuePosition = XMFLOAT2(tileCenter.x + c_letterTileValue_OffsetFromTileCenter.x, tileCenter.y + c_letterTileValue_OffsetFromTileCenter.y);
                    XMVECTOR letterValueSize = m_gameBoardTileValueFont->MeasureString(letterValue);
                    float letterValueScale = 0.45f;
                    m_gameBoardTileValueFont->DrawString(spriteBatch.get(), letterValue, letterValuePosition, Colors::Yellow, 0.0f, XMFLOAT2(0, 0), letterValueScale);
                }
                else
                {
                    // draw the non-alphabetic texture
                }
            }

            // draw word arrows
            auto trackerTile = GetWordTrackerTile(XMUINT2(i, j));
            if (trackerTile.m_wordRight)
            {
                XMFLOAT2 horizontalWordLinkerCenter = XMFLOAT2(c_wordLinkFirstHorizontal_Center.x + (c_letterTile_CenterOffset.x * i), c_wordLinkFirstHorizontal_Center.y + (c_letterTile_CenterOffset.y * j));
                spriteBatch->Draw(m_hortizontalWordLinker->GetResourceViewTemporary(), horizontalWordLinkerCenter, nullptr, Colors::White, 0.0f, XMFLOAT2(float(m_hortizontalWordLinker->Width() / 2), float(m_hortizontalWordLinker->Height() / 2)));
            }
            if (trackerTile.m_wordDown)
            {
                XMFLOAT2 verticalWordLinkerCenter = XMFLOAT2(c_wordLinkFirstVertical_Center.x + (c_letterTile_CenterOffset.x * i), c_wordLinkFirstVertical_Center.y + (c_letterTile_CenterOffset.y * j));
                spriteBatch->Draw(m_verticalWordLinker->GetResourceViewTemporary(), verticalWordLinkerCenter, nullptr, Colors::White, 0.0f, XMFLOAT2(float(m_verticalWordLinker->Width() / 2), float(m_verticalWordLinker->Height() / 2)));
            }
        }
    }

    // Draw input control help
    bool displayGamepadControls = true;

    // Draw game debug log
    auto logPosition = XMFLOAT2(c_debugLog_Region.X, c_debugLog_Region.Y);
    size_t maxDisplayLines = static_cast<size_t>(c_debugLog_Region.Height / m_logFontHeight);
    auto logIndexSize = Log::g_displayLog.size();
    auto logIndexBegin = size_t(0);
    auto logIndexEnd = logIndexSize - 1;

    if (m_logLineBegin > 0)
    {
        // no auto-scrolling, display starting at requested line number
        logIndexBegin = m_logLineBegin - 1;
        logIndexEnd = std::min(logIndexEnd, logIndexBegin + maxDisplayLines - 1);
    }
    else if (logIndexSize > maxDisplayLines)
    {
        // auto-scroll
        logIndexBegin = logIndexSize - maxDisplayLines;
    }

    for (auto i = logIndexBegin; i <= logIndexEnd; ++i)
    {
        m_logFont->DrawString(spriteBatch.get(), Log::g_displayLog[i].c_str(), logPosition);
        logPosition.y += m_logFontHeight;
    }

    // Draw scroll indicators
    if (logIndexBegin > 0)
    {
        if (displayGamepadControls)
        {
            spriteBatch->Draw(m_logScrollUpControl->GetResourceViewTemporary(), c_debugLogScrollUpIcon_Center, nullptr, Colors::White, 0.0f, XMFLOAT2(float(m_logScrollUpControl->Width() / 2), float(m_logScrollUpControl->Height() / 2)));
        }
        else
        {
            wchar_t logScrollUpKeyString[] = L"PgUp";
            XMVECTOR logScrollUpKeySize = m_gameBoardControlsHelpFont->MeasureString(logScrollUpKeyString);
            XMFLOAT2 logScrollUpKeyOrigin = XMFLOAT2(XMVectorGetX(logScrollUpKeySize) / 2.f, m_gameBoardControlsHelpFont->GetLineSpacing() / 2.f);
            m_gameBoardControlsHelpFont->DrawString(spriteBatch.get(), logScrollUpKeyString, c_debugLogScrollUpIcon_Center, Colors::SandyBrown, 0.0f, logScrollUpKeyOrigin, c_inputControlsHelp_Scale);
        }
    }
    if (m_logLineBegin > 0)
    {
        if (displayGamepadControls)
        {
            spriteBatch->Draw(m_logScrollDownControl->GetResourceViewTemporary(), c_debugLogScrollDownIcon_Center, nullptr, Colors::White, 0.0f, XMFLOAT2(float(m_logScrollDownControl->Width() / 2), float(m_logScrollDownControl->Height() / 2)));
        }
        else
        {
            wchar_t logScrollDownKeyString[] = L"PgDn";
            XMVECTOR logScrollDownKeySize = m_gameBoardControlsHelpFont->MeasureString(logScrollDownKeyString);
            XMFLOAT2 logScrollDownKeyOrigin = XMFLOAT2(XMVectorGetX(logScrollDownKeySize) / 2.f, m_gameBoardControlsHelpFont->GetLineSpacing() / 2.f);
            m_gameBoardControlsHelpFont->DrawString(spriteBatch.get(), logScrollDownKeyString, c_debugLogScrollDownIcon_Center, Colors::SandyBrown, 0.0f, logScrollDownKeyOrigin, c_inputControlsHelp_Scale);
        }
    }

    spriteBatch->End();
}

void GameBoardScreen::ExitScreen(bool immediate)
{
    if (!immediate)
    {
        Game->GameSaveManager.Reset();
    }

    MenuScreen::ExitScreen(immediate);
}

void GameBoardScreen::OnCancel()
{

    if (Game->GameSaveManager.IsActiveBoardDirty())
    {
        int controllingPlayer = GetControllingPlayer();
        winrt::hstring exitConfirmationTitle = L"Exit Without Saving?";
        winrt::hstring exitConfirmationMessage = L"The current game board has not been saved.\nAre you sure you wish to exit?";
        ConfirmChoiceFn exitConfirmationFn = [this](bool exitWithoutSaving)
        {
            if (exitWithoutSaving)
            {
                Log::Write(L"ConfirmChoiceFn: Exit without saving confirmed\n");
                Game->StateManager->SwitchState(GameState::AcquireUser);
                ExitScreen();
            }
            else
            {
                Log::Write(L"ConfirmChoiceFn: User chose not to exit without saving\n");
            }
        };

        Manager()->AddScreen(std::make_shared<ConfirmPopUpScreen>(Manager(), exitConfirmationTitle, exitConfirmationMessage, exitConfirmationFn), controllingPlayer);
    }
    else
    {
        Game->StateManager->SwitchState(GameState::AcquireUser);
        ExitScreen();
    }
}

void GameBoardScreen::ComputeMenuBounds(float viewportWidth, float viewportHeight)
{
    UNREFERENCED_PARAMETER(viewportWidth);
    UNREFERENCED_PARAMETER(viewportHeight);
    m_menuBounds = c_menu_Region;
}

int GameBoardScreen::GetWordScore(std::wstring word)
{
    auto wordLength = word.length();
    if (wordLength < 2)
    {
        return 0;
    }

    auto it = m_wordList->find(word);
    if (it == m_wordList->end())
    {
        return 0;
    }

    int score = 0;
    std::for_each(word.cbegin(), word.cend(), [&score](wchar_t letter)
    {
        score += c_letterValues[letter - L'A'];
    });

    return score;
}

GameBoardScreen::WordTrackerTile& GameBoardScreen::GetWordTrackerTile(const XMUINT2& position)
{
    size_t tile = size_t(position.x + (c_boardWidth * position.y));
    if (tile >= c_boardWidth * c_boardHeight)
    {
        throw std::invalid_argument("position is not valid for the current board size");
    }

    return m_wordTracker[tile];
}

void GameBoardScreen::TrackWord(bool isHorizontal, XMUINT2 startTile, size_t wordLength)
{
    if (isHorizontal)
    {
        auto lastX = startTile.x + wordLength - 2;
        for (auto i = startTile.x; i <= lastX; ++i)
        {
            auto& trackerTile = GetWordTrackerTile(startTile);
            trackerTile.m_wordRight = true;
            startTile.x++;
        }
    }
    else
    {
        auto lastY = startTile.y + wordLength - 2;
        for (auto i = startTile.y; i <= lastY; ++i)
        {
            auto& trackerTile = GetWordTrackerTile(startTile);
            trackerTile.m_wordDown = true;
            startTile.y++;
        }
    }
}

void GameBoardScreen::UpdateLettersRemaining()
{
    m_lettersRemaining.assign(c_letterCounts, c_letterCounts + sizeof(c_letterCounts)/sizeof(c_letterCounts[0]));

    auto gameBoard = Game->GameSaveManager.ActiveBoard();
    for (uint32_t j = 0; j < gameBoard.m_boardHeight; ++j)
    {
        for (uint32_t i = 0; i < gameBoard.m_boardWidth; ++i)
        {
            auto currentTile = gameBoard.GetGameTile(XMUINT2(i, j));
            auto currentLetter = currentTile.m_letter;
            if (currentLetter != 0 && currentTile.m_placed)
            {
                m_lettersRemaining[currentLetter - L'A']--;
            }
        }
    }

    // Has player used too many of any letter?
    m_isOverLimitOnLetters = false;
    for (auto it = m_lettersRemaining.cbegin(); it != m_lettersRemaining.cend(); ++it)
    {
        if (*it < 0)
        {
            m_isOverLimitOnLetters = true;
            break;
        }
    }

}

void GameBoardScreen::UpdateScore()
{
    assert(Game->GameSaveManager.HasActiveBoard());

    m_score = 0;
    if (m_wordList == nullptr || m_wordList->empty())
    {
        return;
    }

    auto gameBoard = Game->GameSaveManager.ActiveBoard();
    int newScore = 0;
    std::wstring currentWord;

    // Reset word tracker
    auto boardTiles = c_boardWidth * c_boardHeight;
    for (uint32_t i = 0; i < boardTiles; ++i)
    {
        m_wordTracker[i].m_wordDown = false;
        m_wordTracker[i].m_wordRight = false;
    }

    // Score the horizontal words
    for (uint32_t j = 0; j < gameBoard.m_boardHeight; ++j)
    {
        for (uint32_t i = 0; i < gameBoard.m_boardWidth; ++i)
        {
            auto currentTile = gameBoard.GetGameTile(XMUINT2(i, j));
            auto currentLetter = currentTile.m_letter;
            if (currentLetter != 0 && currentTile.m_placed)
            {
                currentWord += currentLetter;
            }
            else
            {
                if (!currentWord.empty())
                {
                    auto wordScore = GetWordScore(currentWord);
                    if (wordScore > 0)
                    {
                        auto wordLength = uint32_t(currentWord.length());
                        TrackWord(true, XMUINT2(i - wordLength, j), wordLength);
                    }
                    newScore += wordScore;
                    currentWord.clear();
                }
            }
        }
        if (!currentWord.empty())
        {
            auto wordScore = GetWordScore(currentWord);
            if (wordScore > 0)
            {
                auto wordLength = uint32_t(currentWord.length());
                TrackWord(true, XMUINT2(gameBoard.m_boardWidth - wordLength, j), wordLength);
            }
            newScore += wordScore;
            currentWord.clear();
        }
    }

    // Score the vertical words
    for (uint32_t i = 0; i < gameBoard.m_boardWidth; ++i)
    {
        for (uint32_t j = 0; j < gameBoard.m_boardHeight; ++j)
        {
            auto currentTile = gameBoard.GetGameTile(XMUINT2(i, j));
            auto currentLetter = currentTile.m_letter;
            if (currentLetter != 0 && currentTile.m_placed)
            {
                currentWord += currentLetter;
            }
            else
            {
                if (!currentWord.empty())
                {
                    auto wordScore = GetWordScore(currentWord);
                    if (wordScore > 0)
                    {
                        auto wordLength = uint32_t(currentWord.length());
                        TrackWord(false, XMUINT2(i, j - wordLength), wordLength);
                    }
                    newScore += wordScore;
                    currentWord.clear();
                }
            }
        }
        if (!currentWord.empty())
        {
            auto wordScore = GetWordScore(currentWord);
            if (wordScore > 0)
            {
                auto wordLength = uint32_t(currentWord.length());
                TrackWord(false, XMUINT2(i, gameBoard.m_boardHeight - wordLength), wordLength);
            }
            newScore += wordScore;
            currentWord.clear();
        }
    }

    if (!m_isOverLimitOnLetters)
    {
        m_score = newScore;
    }
}

} // namespace GameSaveSample
