// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

#include "ContentManager.h"
#include "MenuScreen.h"
#include "Texture2D.h"

namespace GameSaveSample
{
    class GameBoardScreen : public MenuScreen
    {
    public:
        GameBoardScreen(const std::shared_ptr<ScreenManager>& screenManager);
	    virtual ~GameBoardScreen();

        virtual void LoadContent() override;
        virtual void Update(float totalTime, float elapsedTime, bool otherScreenHasFocus, bool coveredByOtherScreen) override;
        virtual void HandleInput(const DirectX::InputState& input) override;
        virtual void Draw(float totalTime, float elapsedTime) override;
        virtual void ExitScreen(bool immediate = false) override;

    protected:
	    virtual void OnCancel() override;
        virtual void ComputeMenuBounds(float viewportWidth, float viewportHeight) override;

    private:
        struct WordTrackerTile
        {
            WordTrackerTile() {}
            bool m_wordDown = false;
            bool m_wordRight = false;
        };

        int GetWordScore(std::wstring word);
        WordTrackerTile& GetWordTrackerTile(const DirectX::XMUINT2& position); // returns WordTrackerTile given a zero-based (x, y) position
        void TrackWord(bool isHorizontal, DirectX::XMUINT2 startTile, size_t wordLength);
        void UpdateLettersRemaining();
        void UpdateScore();

        //
        // Assets
        //

        // fonts

        std::shared_ptr<DirectX::SpriteFont>                m_gameBoardControlsHelpFont;
        std::shared_ptr<DirectX::SpriteFont>                m_gameBoardLettersRemainingFont;
        std::shared_ptr<DirectX::SpriteFont>                m_gameBoardMetadataFont;
        std::shared_ptr<DirectX::SpriteFont>                m_gameBoardTileFont;
        std::shared_ptr<DirectX::SpriteFont>                m_gameBoardTileValueFont;
        std::shared_ptr<DirectX::SpriteFont>                m_gameBoardTitleFont;
        std::shared_ptr<DirectX::SpriteFont>                m_logFont;
        float                                               m_logFontHeight;

        // textures

        std::shared_ptr<DirectX::Texture2D>                 m_background;
        std::shared_ptr<DirectX::Texture2D>                 m_clearLetterControl;
        std::shared_ptr<DirectX::Texture2D>                 m_cursor;
        std::shared_ptr<DirectX::Texture2D>                 m_hortizontalWordLinker;
        std::shared_ptr<DirectX::Texture2D>                 m_logScrollDownControl;
        std::shared_ptr<DirectX::Texture2D>                 m_logScrollUpControl;
        std::shared_ptr<DirectX::Texture2D>                 m_menuControl;
        std::shared_ptr<DirectX::Texture2D>                 m_moveCursorControl;
        std::shared_ptr<DirectX::Texture2D>                 m_saveSlotLeftControl;
        std::shared_ptr<DirectX::Texture2D>                 m_saveSlotRightControl;
        std::vector<std::shared_ptr<DirectX::Texture2D>>    m_saveSlotNumbers;
        std::vector<std::shared_ptr<DirectX::Texture2D>>    m_saveSlotActiveNumbers;
        std::shared_ptr<DirectX::Texture2D>                 m_swapLetterControl;
        std::shared_ptr<DirectX::Texture2D>                 m_verticalWordLinker;

        //
        // Game State
        //

        DirectX::XMUINT2                                    m_cursorPosition; // 0-based (x, y) position on board
        uint32_t                                            m_firstSlotToDisplay;
		bool												m_isGameSaveManagerInitialized;
        bool                                                m_isOverLimitOnLetters;
        std::vector<int>                                    m_lettersRemaining;
        size_t                                              m_logLineBegin; // set to non-zero value to stop auto-scrolling
        float                                               m_logScrollDownDelay;
        float                                               m_logScrollUpDelay;
        int                                                 m_score;
        float                                               m_tileScrollDownDelay;
        float                                               m_tileScrollUpDelay;
        WordList                                            m_wordList;
        bool                                                m_wordListLoaded;
        std::vector<WordTrackerTile>                        m_wordTracker;
    };
}
