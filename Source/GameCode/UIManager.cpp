
#define UI_MANAGER_CPP

#include <string.h>
#include <stdio.h>
#include <windows.h>											// Header File For Windows
#include <gl\gl.h>											    // Header File For The OpenGL32 Library
#include <gl\glu.h>												// Header File For The GLu32 Library
#include <assert.h>
#include "glut.h"
#include "..\\Framework\\baseTypes.h"
#include "..\\Framework\\openGLFramework.h"

#include "SOIL.h"

#include <string>
#include "Utilities.h"
#include "UIManager.h"

#include <queue>
#include "EventManager.h"

#include "StatsManager.h"

#define TEXT_SPRITESHEET    "Assets\\Images\\Text.png"
#define LIVES_SPRITESHEET   "Assets\\Images\\LifeSymbol.png"
#define MENU_IMAGE          "Assets\\Images\\Menu.png"

#define UI_ELEM_SPRITE_SIZE     32
#define UI_ELEM_OFFSET_FACTOR   UI_ELEM_SPRITE_SIZE*3.25f

#define NUM_TEXT_ROWS       6
#define NUM_TEXT_COLS       16
#define TEXT_ROW_SIZE       1.0f/NUM_TEXT_ROWS
#define TEXT_COL_SIZE       1.0f/NUM_TEXT_COLS

// in-game ui element positions
#define READY_TEXT          "READY!"
#define READY_TEXT_LEN      6
#define READY_TEXT_POS_X    -312
#define READY_TEXT_POS_Y    -260

#define PLAYER_TEXT         "PLAYER ONE"
#define PLAYER_TEXT_LEN     10
#define PLAYER_TEXT_POS_X   -572
#define PLAYER_TEXT_POS_Y   364

#define ONEUP_TEXT          "1UP"
#define ONEUP_TEXT_LEN      3
#define ONEUP_TEXT_POS_X    -1196
#define ONEUP_TEXT_POS_Y    1768
#define SCORE_POS_X         -1196
#define SCORE_POS_Y         1666

#define HIGHSCORE_TEXT          "HIGH SCORE"
#define HIGHSCORE_TEXT_LEN      10
#define HIGHSCORE_TEXT_POS_X    0
#define HIGHSCORE_TEXT_POS_Y    1768
#define HIGHSCORE_POS_X         0
#define HIGHSCORE_POS_Y         1666

#define LIVES_POS_X         -1196
#define LIVES_POS_Y         -1768

// menu ui element positions
#define MENU_HALF_W             1456.0f
#define MENU_HALF_H             1872.0f

#define MENU_SCORE_POS_X        100
#define MENU_SCORE_POS_Y        275
#define MENU_HIGHSCORE_POS_X    100
#define MENU_HIGHSCORE_POS_Y    75

UIManager* UIManager::_instance = nullptr;

UIManager::UIManager() :
    mTextSpritesheetID(0),
    mLivesSpritesheetID(0),
    mMenuImageID(0)
{
}

//-----------------------------------------------------------------------------------------

UIManager* UIManager::Instance()
{
    if(_instance == nullptr)
    {
        _instance = new UIManager();
    }

    return _instance;
}

//-----------------------------------------------------------------------------------------

const UIManager* UIManager::ConstInstace()
{
    if(_instance == nullptr)
    {
        _instance = new UIManager();
    }

    return _instance;
}

//-----------------------------------------------------------------------------------------

void UIManager::init()
{
    mTextSpritesheetID = SOIL_load_OGL_texture(TEXT_SPRITESHEET, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
    assert(mTextSpritesheetID > 0);

    mLivesSpritesheetID = SOIL_load_OGL_texture(LIVES_SPRITESHEET, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
    assert(mLivesSpritesheetID > 0);

    mMenuImageID = SOIL_load_OGL_texture(MENU_IMAGE, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
    assert(mMenuImageID > 0);

    setUIState(GameUtil::UIState::UI_MENU);
    mScoreString = "00";

    // initialize score string using the stats manager's value
    int32_t highScore = StatsManager::ConstInstace()->getHighScore();
    if(highScore > 0)
    {
        char8_t buffer[16];
        memset(buffer, 0x00, 16);
        int32_t index = 0;

        updateScoreStringHelper(buffer, highScore, index);
        mHighScoreString = buffer;
    }
    else
    {
        mHighScoreString = "00";
    }
}

//-----------------------------------------------------------------------------------------

void UIManager::reset()
{
    mScoreString = "00";
}

//-----------------------------------------------------------------------------------------

void UIManager::update()
{
    // if in the menu, listen for spacebar to start game
    if(mCurrentState == GameUtil::UIState::UI_MENU)
    {
        if(g_keys->keyDown[VK_SPACE])
        {
            // switching ui state and posting event to begin start-game-timer
            const EventManager* emManager = EventManager::ConstInstace();
            emManager->postEvent(EventManager::GameEvent::START_FROM_MENU);
            emManager->postSoundEvent(EventManager::SoundEvent::SOUND_START);

            setUIState(GameUtil::UIState::UI_PREGAME_1);
        }
    }
}

//-----------------------------------------------------------------------------------------

void UIManager::draw()
{
    switch(mCurrentState)
    {
        case GameUtil::UIState::UI_MENU:
            drawGameOverScreen();
            break;

        case GameUtil::UIState::UI_PREGAME_1:
            drawGameUI();
            drawPlayerOneText();
            drawReadyText();
            break;

        case GameUtil::UIState::UI_PREGAME_2:
            drawGameUI();
            drawReadyText();
            break;

        case GameUtil::UIState::UI_GAME:
            drawGameUI();
            break;
    }
}

//-----------------------------------------------------------------------------------------

void UIManager::shutdown()
{
}

//-----------------------------------------------------------------------------------------

void UIManager::drawGameOverScreen()
{
    // display scores on menu
    drawText(mScoreString.c_str(), mScoreString.length(), MENU_SCORE_POS_X, MENU_SCORE_POS_Y);
    drawText(mHighScoreString.c_str(), mHighScoreString.length(), MENU_HIGHSCORE_POS_X, MENU_HIGHSCORE_POS_Y);

    // draw game over screen
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, mMenuImageID);
    glBegin(GL_QUADS);
    
    Coord2D vertTR, vertBL, textureCoord, textureSize;

    vertTR.x = MENU_HALF_W;
    vertTR.y = -MENU_HALF_H;
    vertBL.x = -MENU_HALF_W;
    vertBL.y = MENU_HALF_H;

    textureCoord.x = 0.0f;
    textureCoord.y = 0.0f;
    textureSize.x = 1.0f;
    textureSize.y = 1.0f;

    glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

    GameUtil::Utilities::drawSprite(vertTR, vertBL, textureCoord, textureSize);

    glEnd();
}

//-----------------------------------------------------------------------------------------

void UIManager::drawPlayerOneText()
{
    // cyan
    glColor4ub(0x00, 0xFF, 0xFF, 0xFF);
    drawText(PLAYER_TEXT, PLAYER_TEXT_LEN, PLAYER_TEXT_POS_X, PLAYER_TEXT_POS_Y);
}

//-----------------------------------------------------------------------------------------

void UIManager::drawReadyText()
{
    // yellow
    glColor4ub(0xFF, 0xFF, 0x00, 0xFF);
    drawText(READY_TEXT, READY_TEXT_LEN, READY_TEXT_POS_X, READY_TEXT_POS_Y);
}

//-----------------------------------------------------------------------------------------

void UIManager::drawGameUI()
{
    // draw text elements -- score and high score
    drawText(ONEUP_TEXT, ONEUP_TEXT_LEN, ONEUP_TEXT_POS_X, ONEUP_TEXT_POS_Y);
    drawText(mScoreString.c_str(), mScoreString.length(), SCORE_POS_X, SCORE_POS_Y);

    drawText(HIGHSCORE_TEXT, HIGHSCORE_TEXT_LEN, HIGHSCORE_TEXT_POS_X, HIGHSCORE_TEXT_POS_Y);
    drawText(mHighScoreString.c_str(), mHighScoreString.length(), HIGHSCORE_POS_X, HIGHSCORE_POS_Y);

    // draw picture elements -- lives symbols
    drawLives();
}

//-----------------------------------------------------------------------------------------

void UIManager::drawText(const char8_t* text, const int32_t length, const int32_t x, const int32_t y)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, mTextSpritesheetID);
    glBegin(GL_QUADS);

    Coord2D vertTR, vertBL, textureCoord, textureSize;
    int32_t currCharInt;

    for(int32_t i = 0; i < length; ++i)
    {
        currCharInt = (int32_t)(text[i] - ' ');

        vertBL.x = x + (UI_ELEM_OFFSET_FACTOR * i);
        vertBL.y = y + UI_ELEM_OFFSET_FACTOR;
        vertTR.x = vertBL.x + UI_ELEM_OFFSET_FACTOR;
        vertTR.y = y;

        textureCoord.x = (currCharInt % NUM_TEXT_COLS) * TEXT_COL_SIZE;
        textureCoord.y = 1.0f - ((currCharInt / NUM_TEXT_COLS) * TEXT_ROW_SIZE) - TEXT_ROW_SIZE;
        textureSize.x = TEXT_COL_SIZE;
        textureSize.y = TEXT_ROW_SIZE;

        GameUtil::Utilities::drawSprite(vertTR, vertBL, textureCoord, textureSize);
    }

    glEnd();
}

//-----------------------------------------------------------------------------------------

void UIManager::drawLives()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, mLivesSpritesheetID);
    glBegin(GL_QUADS);

    Coord2D vertTR, vertBL, textureCoord, textureSize;

    int32_t lives = StatsManager::ConstInstace()->getLives();
    for(int32_t i = 0; i < lives; ++i)
    {
        vertBL.x = LIVES_POS_X + (UI_ELEM_OFFSET_FACTOR * i);
        vertBL.y = LIVES_POS_Y + UI_ELEM_OFFSET_FACTOR;
        vertTR.x = vertBL.x + UI_ELEM_OFFSET_FACTOR;
        vertTR.y = LIVES_POS_Y;

        textureCoord.x = 0.0f;
        textureCoord.y = 0.0f;
        textureSize.x = 1.0f;
        textureSize.y = 1.0f;

        glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

        GameUtil::Utilities::drawSprite(vertTR, vertBL, textureCoord, textureSize);
    }

    glEnd();
}

//-----------------------------------------------------------------------------------------

void UIManager::updateScoreStrings()
{
    const StatsManager* smInstance = StatsManager::ConstInstace();
    
    // a "perfect score" in pacman is 3333360, so we are more than safe using a buffer of size 16 for the score string
    char8_t buffer[16];
    
    // update score string
    memset(buffer, 0x00, 16);
    int32_t index = 0;

    int32_t score = smInstance->getScore();
    updateScoreStringHelper(buffer, score, index);
    mScoreString = buffer;

    // then update high score string (possibly new high score)
    memset(buffer, 0x00, 16);
    index = 0;

    int32_t highScore = smInstance->getHighScore();
    updateScoreStringHelper(buffer, highScore, index);
    mHighScoreString = buffer;
}

//-----------------------------------------------------------------------------------------

void UIManager::updateScoreStringHelper(char8_t* buffer, int32_t num, int32_t& index)
{
    if(num >= 10)
    {
        updateScoreStringHelper(buffer, num / 10, index);
        index++;
    }
    buffer[index] = (char8_t)((num % 10) + '0');
}

//-----------------------------------------------------------------------------------------

void UIManager::setUIState(const GameUtil::UIState state)
{
    mCurrentState = state;
}
