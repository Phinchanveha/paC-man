
#define TILE_CPP

#include <stdio.h>
#include <windows.h>											// Header File For Windows
#include <gl\gl.h>											    // Header File For The OpenGL32 Library
#include <gl\glu.h>												// Header File For The GLu32 Library
#include <assert.h>
#include "..\\Framework\\baseTypes.h"
#include "..\\Framework\\ShapeDraw.h"

#include <map>
#include "Utilities.h"
#include "Tile.h"
#include "BoardManager.h"

#include "Character.h"
#include "Pacman.h"
#include "Ghost.h"
#include "GhostRed.h"
#include "GhostPink.h"
#include "GhostBlue.h"
#include "GhostOrange.h"
#include "CharacterManager.h"

#include <queue>
#include "EventManager.h"

#define TILE_SPRITE_SIZE            32
#define TILE_SPRITE_OFFSET_FACTOR   TILE_SPRITE_SIZE*3.25f
#define HALF_TILE_SPRITE_SIZE       TILE_SPRITE_OFFSET_FACTOR/2.0f

#define PELLET_SIZE             5.0f*3.25f/2.0f
#define POWER_PELLET_SIZE       15.0f*3.25f/2.0f

#define TILE_ANIM_CHUNK_SIZE    1.0f/NUM_TILE_ANIM_CHUNKS
#define TILE_ROW_SIZE           1.0f/NUM_TILE_TEXTURE_ROWS
#define TILE_COL_SIZE           1.0f/(NUM_TILE_TEXTURE_COLS*NUM_TILE_ANIM_CHUNKS)

#define TILE_FRAME_DELTA_TIME   250

Tile::Tile(const Coord2D& textureOffset, const Coord2D& index, const Coord2D& boardPosition, const TileTraverseType traverse, const TilePelletType pellet) :
    mTraverseType(traverse),
    mPelletType(pellet),
    mNeighbors(new Neighbors())
{
    mBoardIndex.x = index.x;
    mBoardIndex.y = index.y;

    mTextureOffset.x = textureOffset.x;
    mTextureOffset.y = textureOffset.y;

    // y position is reversed, need to compensate
    mPosition.x = boardPosition.x + (float)(index.x - NUM_BOARD_COLS / 2.0f) * TILE_SPRITE_OFFSET_FACTOR;
    mPosition.y = boardPosition.y - (float)(index.y - NUM_BOARD_ROWS / 2.0f) * TILE_SPRITE_OFFSET_FACTOR;

    // set radius of the pellet based on our pellet type
    switch(mPelletType)
    {
        case TilePelletType::PELLET:
            mPelletSize = PELLET_SIZE;
            break;

        case TilePelletType::POWER_PELLET:
            mPelletSize = POWER_PELLET_SIZE;
            break;

        default:
            mPelletSize = 0.0f;
            break;
    }
}

//-----------------------------------------------------------------------------------------

void Tile::init()
{
}

//-----------------------------------------------------------------------------------------

void Tile::reset()
{
    // reset collection and animation variables
    mIsPelletCollected = false;
    mIsFlashing = false;

    mAnimFrame = 0;
    mTimeOnCurrFrame = 0;
}

//-----------------------------------------------------------------------------------------

void Tile::updateTile(DWORD milliseconds)
{
    if(mIsFlashing)
    {
        // if delta time has passed, toggle the animation frame
        mTimeOnCurrFrame += milliseconds;
        if(mTimeOnCurrFrame >= TILE_FRAME_DELTA_TIME)
        {
            mTimeOnCurrFrame = 0;
            mAnimFrame = (mAnimFrame + 1) % 2;
        }
    }
}

//-----------------------------------------------------------------------------------------

void Tile::drawTile()
{
    Coord2D vertTR, vertBL, textureCoord, textureSize;

    // offset vert positions so the tile's position is its center
    vertTR.x = (mPosition.x + HALF_TILE_SPRITE_SIZE);
    vertTR.y = (mPosition.y - HALF_TILE_SPRITE_SIZE);
    vertBL.x = (mPosition.x - HALF_TILE_SPRITE_SIZE);
    vertBL.y = (mPosition.y + HALF_TILE_SPRITE_SIZE);
    
    textureCoord.x = mTextureOffset.x + (mAnimFrame * TILE_ANIM_CHUNK_SIZE);
    textureCoord.y = mTextureOffset.y;
    textureSize.x = TILE_COL_SIZE;
    textureSize.y = TILE_ROW_SIZE;

    glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

    GameUtil::Utilities::drawSprite(vertTR, vertBL, textureCoord, textureSize);
}

//-----------------------------------------------------------------------------------------

void Tile::drawPellet()
{
    if(mPelletType != TilePelletType::NO_PELLET && !mIsPelletCollected)
    {
        Coord2D vertTR, vertBL, textureCoord, textureSize;

        vertTR.x = mPosition.x + mPelletSize;
        vertTR.y = mPosition.y - mPelletSize;
        vertBL.x = mPosition.x - mPelletSize;
        vertBL.y = mPosition.y + mPelletSize;

        textureCoord.x = 0.0f;
        textureCoord.y = 0.0f;
        textureSize.x = 1.0f;
        textureSize.y = 1.0f;

        glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

        GameUtil::Utilities::drawSprite(vertTR, vertBL, textureCoord, textureSize);
    }
}

//-----------------------------------------------------------------------------------------

void Tile::shutdown()
{
    delete mNeighbors;
}

//-----------------------------------------------------------------------------------------

bool8_t Tile::collectPellet()
{
    bool8_t result = false;

    // if this tile has a pellet that has not yet been collected...
    if(mPelletType != TilePelletType::NO_PELLET && !mIsPelletCollected)
    {
        mIsPelletCollected = true;
        result = true;

        const EventManager* emInstance = EventManager::ConstInstace();
        emInstance->postSoundEvent(EventManager::SoundEvent::SOUND_PELLET);

        if(mPelletType == TilePelletType::POWER_PELLET)
        {
            // scare ghosts if collected power pellet
            emInstance->postEvent(EventManager::GameEvent::POINTS_POWER_PELLET);
            emInstance->postEvent(EventManager::GameEvent::SCARE_GHOSTS);
        }
        else
        {
            emInstance->postEvent(EventManager::GameEvent::POINTS_PELLET);
        }
    }

    // returns whether we collected a pellet or not
    return result;
}

//-----------------------------------------------------------------------------------------

void Tile::activateFlash()
{
    mIsFlashing = true;
}

//-----------------------------------------------------------------------------------------

void Tile::setNeighbors(const Tile* up, const Tile* left, const Tile* down, const Tile* right)
{
    // the order of the neighbors is important...
    //      if two or more neighbors would place a ghost equidistant to its target,
    //      the order of the neighbors is the priority with which the ghost chooses its next tile
    mNeighbors->insert(NeighborsPair(GameUtil::BoardDirection::UP, up));
    mNeighbors->insert(NeighborsPair(GameUtil::BoardDirection::LEFT, left));
    mNeighbors->insert(NeighborsPair(GameUtil::BoardDirection::DOWN, down));
    mNeighbors->insert(NeighborsPair(GameUtil::BoardDirection::RIGHT, right));
}

//-----------------------------------------------------------------------------------------

const Tile* Tile::getNeighbor(GameUtil::BoardDirection neighborID) const
{
    return mNeighbors->at(neighborID);
}

//-----------------------------------------------------------------------------------------

const Coord2D& Tile::getPosition() const
{
    return mPosition;
}

//-----------------------------------------------------------------------------------------

const Coord2D& Tile::getBoardIndex() const
{
    return mBoardIndex;
}

//-----------------------------------------------------------------------------------------

const Tile::TileTraverseType Tile::getTraverseType() const
{
    return mTraverseType;
}

//-----------------------------------------------------------------------------------------

const bool Tile::isTeleporter() const
{
    return mTraverseType == Tile::TileTraverseType::TELEPORT;
}

//-----------------------------------------------------------------------------------------

const bool Tile::isTunnel() const
{
    // teleporter tiles are also part of the tunnel
    return mTraverseType == Tile::TileTraverseType::TUNNEL || mTraverseType == Tile::TileTraverseType::TELEPORT;
}

//-----------------------------------------------------------------------------------------

const bool Tile::isTraversable() const
{
    // traversible if not marked "non-traversable" or "ghost-traversable"
    return mTraverseType != Tile::TileTraverseType::NON_TRAVERSABLE && mTraverseType != Tile::TileTraverseType::GHOST_TRAVERSABLE;
}

//-----------------------------------------------------------------------------------------

const bool Tile::isGhostTraversable() const
{
    // ghost traversible if not marked "non-traversable"
    return mTraverseType != Tile::TileTraverseType::NON_TRAVERSABLE;
}

const bool Tile::isPelletPresent() const
{
    return mPelletType != Tile::TilePelletType::NO_PELLET && !mIsPelletCollected;
}

//-----------------------------------------------------------------------------------------

const float Tile::getSquareDistToTile(const Tile* other) const
{
    Coord2D otherPos = other->getPosition();
    return GameUtil::Utilities::squareDistance(otherPos.x, mPosition.x, otherPos.y, mPosition.y);
}
