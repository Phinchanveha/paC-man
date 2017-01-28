
#define FRUIT_MANAGER_CPP

#include <stdio.h>
#include <windows.h>											// Header File For Windows
#include <gl\gl.h>											    // Header File For The OpenGL32 Library
#include <gl\glu.h>												// Header File For The GLu32 Library
#include <assert.h>
#include "..\\Framework\\baseTypes.h"

#include "soil.h"

#include <map>
#include "Utilities.h"
#include "Tile.h"

#include "FruitManager.h"

#include "Character.h"
#include "Pacman.h"
#include "Ghost.h"
#include "GhostRed.h"
#include "GhostPink.h"
#include "GhostBlue.h"
#include "GhostOrange.h"
#include "BoardManager.h"
#include "CharacterManager.h"

#include <queue>
#include "EventManager.h"

#include "StatsManager.h"

#define FRUIT_SPRITESHEET "Assets\\Images\\FruitSpritesheet.png"


#define FRUIT_DURATION  10000

#define NUM_FRUIT                       8
#define FRUIT_SPRITE_SIZE               32
#define FRUIT_SPRITE_OFFSET_FACTOR      FRUIT_SPRITE_SIZE*3.25f
#define HALF_FRUIT_SPRITE_SIZE          FRUIT_SPRITE_OFFSET_FACTOR/2.0f

#define FRUIT_ROW_SIZE                  1.0f
#define FRUIT_COL_SIZE                  1.0f/NUM_FRUIT

#define COLLECTED_FRUIT_POS_X   260
#define COLLECTED_FRUIT_POS_Y   -1768

FruitManager* FruitManager::_instance = nullptr;

FruitManager::FruitManager() :
    mCollectedFruit(new CollectedFruit())
{
}

//-----------------------------------------------------------------------------------------

FruitManager* FruitManager::Instance()
{
    if(_instance == nullptr)
    {
        _instance = new FruitManager();
    }

    return _instance;
}

//-----------------------------------------------------------------------------------------

const FruitManager* FruitManager::ConstInstace()
{
    // returning a const-safe instance of the FruitManager
    //      this way, the caller can't do anything it shouldn't to the manager
    if(_instance == nullptr)
    {
        _instance = new FruitManager();
    }

    return _instance;
}

//-----------------------------------------------------------------------------------------

void FruitManager::init()
{
    mFruitSpritesheetID = SOIL_load_OGL_texture(FRUIT_SPRITESHEET, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
    assert(mFruitSpritesheetID > 0);

    // init tile position
    mFruitTile = BoardManager::ConstInstace()->getFruitTile();
    Coord2D tilePos = mFruitTile->getPosition();

    mPosition.x = tilePos.x;
    mPosition.y = tilePos.y;

    // build map of collected fruit -- init to false
    GameUtil::FruitType type = GameUtil::FruitType::FRUIT_TYPE_INVAL;
    for(uint32_t fruitInt = GameUtil::FruitType::FRUIT_KEY; fruitInt != GameUtil::FruitType::FRUIT_TYPE_INVAL; ++fruitInt)
    {
        type = static_cast<GameUtil::FruitType>(fruitInt);
        mCollectedFruit->insert(CollectedFruitPair(type, false));
    }
}

//-----------------------------------------------------------------------------------------

void FruitManager::reset()
{
    // full reset from game over
    mTimer = 0;
    mIsVisible = false;
    mIsPaused = true;
    mPhase = GameUtil::FruitPhase::PRE_FRUIT;

    // reset collected fruit to false, then set cherry to true (game starts with cherry displaying)
    for(auto iter = mCollectedFruit->begin(); iter != mCollectedFruit->end(); ++iter)
    {
        iter->second = false;
    }
    mCollectedFruit->at(GameUtil::FruitType::FRUIT_CHERRY) = true;

    // reset the fruit type to cherry for the new game
    mFruitType = GameUtil::FruitType::FRUIT_CHERRY;
}

//-----------------------------------------------------------------------------------------

void FruitManager::resetFromDeath()
{
    // resets fruit timer from pacman death
    mTimer = 0;
    mIsVisible = false;
    mIsPaused = true;
}

//-----------------------------------------------------------------------------------------

void FruitManager::nextLevel()
{
    // reset fruit collection variables
    //      we don't reset the collected fruit map unless we're completely resetting
    mTimer = 0;
    mIsVisible = false;
    mIsPaused = true;
    mPhase = GameUtil::FruitPhase::PRE_FRUIT;

    // save the fruit type for the current level
    mFruitType = StatsManager::ConstInstace()->getFruitType();
}

//-----------------------------------------------------------------------------------------

void FruitManager::updateFruit(DWORD milliseconds)
{
    if(mIsVisible && !mIsPaused)
    {
        const CharacterManager* cmInstance = CharacterManager::ConstInstance();

        const Character::MovementState movementState = cmInstance->getCharacterMovementState(GameUtil::CharacterID::PACMAN);
        if(movementState == Character::AT_TILE)
        {
            // post fruit collected event and mark fruit as collected if pacman's tile is the fruit tile
            const Tile* pacmanTile = cmInstance->getCharacterCurrentTile(GameUtil::CharacterID::PACMAN);
            if(pacmanTile == mFruitTile)
            {
                const EventManager* emManager = EventManager::ConstInstace();
                emManager->postEvent(EventManager::GameEvent::POINTS_FRUIT);
                emManager->postSoundEvent(EventManager::SoundEvent::SOUND_FRUIT);

                mCollectedFruit->at(mFruitType) = true;
                deactivateFruit();
            }
        }

        // update timer
        if(mTimer > 0)
        {
            mTimer -= milliseconds;
        }
        else
        {
            deactivateFruit();
        }
    }
}

//-----------------------------------------------------------------------------------------

void FruitManager::drawFruit()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, mFruitSpritesheetID);
    glBegin(GL_QUADS);

    drawActiveFruit();
    drawCollectedFruit();

    glEnd();
}

//-----------------------------------------------------------------------------------------

void FruitManager::drawActiveFruit()
{
    if(mIsVisible)
    {
        Coord2D vertTR, vertBL, textureCoord, textureSize;

        vertTR.x = (mPosition.x + HALF_FRUIT_SPRITE_SIZE);
        vertTR.y = (mPosition.y - HALF_FRUIT_SPRITE_SIZE);
        vertBL.x = (mPosition.x - HALF_FRUIT_SPRITE_SIZE);
        vertBL.y = (mPosition.y + HALF_FRUIT_SPRITE_SIZE);

        textureCoord.x = mFruitType * FRUIT_COL_SIZE;
        textureCoord.y = 0.0f;
        textureSize.x = FRUIT_COL_SIZE;
        textureSize.y = 1.0f;

        glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

        GameUtil::Utilities::drawSprite(vertTR, vertBL, textureCoord, textureSize);
    }
}

//-----------------------------------------------------------------------------------------

void FruitManager::drawCollectedFruit()
{
    Coord2D vertTR, vertBL, textureCoord, textureSize;
    int32_t i = 0;

    for(auto iter = mCollectedFruit->begin(); iter != mCollectedFruit->end(); ++iter, ++i)
    {
        vertBL.x = COLLECTED_FRUIT_POS_X + (FRUIT_SPRITE_OFFSET_FACTOR * i);
        vertBL.y = COLLECTED_FRUIT_POS_Y + FRUIT_SPRITE_OFFSET_FACTOR;
        vertTR.x = vertBL.x + FRUIT_SPRITE_OFFSET_FACTOR;
        vertTR.y = COLLECTED_FRUIT_POS_Y;

        textureCoord.x = iter->first * FRUIT_COL_SIZE;
        textureCoord.y = 0.0f;
        textureSize.x = FRUIT_COL_SIZE;
        textureSize.y = 1.0f;

        if(iter->second)
        {
            GameUtil::Utilities::drawSprite(vertTR, vertBL, textureCoord, textureSize);
        }
    }
}

//-----------------------------------------------------------------------------------------

void FruitManager::shutdown()
{
    delete mCollectedFruit;
}

//-----------------------------------------------------------------------------------------

void FruitManager::activateFruit()
{
    // activate enter next phase while there is a next phase
    GameUtil::FruitPhase nextPhase = (GameUtil::FruitPhase)(mPhase + 1);
    if(nextPhase != GameUtil::FruitPhase::FRUIT_INVAL)
    {
        mTimer = FRUIT_DURATION;
        mPhase = nextPhase;
        mIsVisible = true;
    }
}

//-----------------------------------------------------------------------------------------

void FruitManager::deactivateFruit()
{
    mIsVisible = false;
    mTimer = 0;
}

//-----------------------------------------------------------------------------------------

void FruitManager::pauseFruit()
{
    mIsPaused = true;
}

//-----------------------------------------------------------------------------------------

void FruitManager::unpauseFruit()
{
    mIsPaused = false;
}

//-----------------------------------------------------------------------------------------

const GameUtil::FruitPhase FruitManager::getPhase() const
{
    return mPhase;
}
