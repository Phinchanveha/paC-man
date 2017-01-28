
#define CHARACTER_CPP

#include <stdio.h>
#include <windows.h>											// Header File For Windows
#include <gl\gl.h>											    // Header File For The OpenGL32 Library
#include <gl\glu.h>												// Header File For The GLu32 Library
#include <assert.h>
#include "..\\Framework\\baseTypes.h"

#include <map>
#include "Utilities.h"
#include "Tile.h"
#include "Character.h"

#define BASE_SPEED 0.56f

#define FRAME_DELTA_TIME     100

Character::Character(const Tile* spawnTile, const GameUtil::CharacterID id) :
    mCharacterID(id),
    mSpawnTile(spawnTile),
    mBaseSpeed(BASE_SPEED)
{
}

//-----------------------------------------------------------------------------------------

void Character::init()
{
}

//-----------------------------------------------------------------------------------------

void Character::reset()
{
    // reset bools
    mIsPaused = true;
    mIsVisible = false;
    mIsAlive = true;
    mDidTeleport = false;

    // reset animation
    mAnimChunkID = (GameUtil::AnimChunkID)mCharacterID;
    mAnimFrame = 0;
    mTimeOnCurrFrame = 0;
    mFrameDeltaTime = FRAME_DELTA_TIME;

    // reset tiles and position
    mCurrTile = mNextTile = mPrevTile = mSpawnTile;
    Coord2D spawnPos = mSpawnTile->getPosition();
    mPosition.x = spawnPos.x;
    mPosition.y = spawnPos.y;

    // reset movement states
    mFacingDirection = GameUtil::BoardDirection::LEFT;
    mMovementState = MovementState::AT_TILE;
}

//-----------------------------------------------------------------------------------------

void Character::resetFromDeath()
{
    // reset bools
    mIsPaused = true;
    mIsVisible = false;
    mIsAlive = true;
    mDidTeleport = false;

    // reset animation
    mAnimChunkID = (GameUtil::AnimChunkID)mCharacterID;
    mAnimFrame = 0;
    mTimeOnCurrFrame = 0;
    mFrameDeltaTime = FRAME_DELTA_TIME;

    // reset tiles and position
    mCurrTile = mNextTile = mPrevTile = mSpawnTile;
    Coord2D spawnPos = mSpawnTile->getPosition();
    mPosition.x = spawnPos.x;
    mPosition.y = spawnPos.y;

    // reset movement states
    mFacingDirection = GameUtil::BoardDirection::LEFT;
    mMovementState = MovementState::AT_TILE;
}

//-----------------------------------------------------------------------------------------

void Character::updateTimers(DWORD milliseconds)
{
}

//-----------------------------------------------------------------------------------------

void Character::updateMovement(DWORD milliseconds)
{
    if(!mIsPaused)
    {
        switch (mMovementState)
        {
            case MovementState::MOVING:
                handleMovement(milliseconds);
                break;

            // if at a tile, run the function to select the next tile
            //      this is virtual -- how the player selects and how the ghosts select are different
            case MovementState::AT_TILE:
                chooseNextTile();
                break;

            default:
                break;
        }
    }
}

//-----------------------------------------------------------------------------------------

void Character::updateAnimation(DWORD milliseconds)
{
    if(!mIsPaused)
    {
        // update the time spent on the current frame
        mTimeOnCurrFrame += milliseconds;
        if(mTimeOnCurrFrame >= mFrameDeltaTime)
        {
            // if the delta time has passed, increment our current frame
            mTimeOnCurrFrame = 0;
            mAnimFrame++;

            // if the frame is out of bounds, wrap around
            if(mAnimFrame >= NUM_ANIM_FRAMES)
            {
                mAnimFrame = 0;
            }
        }
    }
}

//-----------------------------------------------------------------------------------------

void Character::drawCharacter()
{
    if(mIsVisible)
    {
        Coord2D vertTR, vertBL, textureCoord, textureSize;

        vertTR.x = (mPosition.x + HALF_CHARACTER_SPRITE_SIZE);
        vertTR.y = (mPosition.y - HALF_CHARACTER_SPRITE_SIZE);
        vertBL.x = (mPosition.x - HALF_CHARACTER_SPRITE_SIZE);
        vertBL.y = (mPosition.y + HALF_CHARACTER_SPRITE_SIZE);

        textureCoord.x = (mAnimChunkID * CHARACTER_CHUNK_SIZE) + (mFacingDirection * CHARACTER_COL_SIZE);
        textureCoord.y = mAnimFrame * CHARACTER_ROW_SIZE;
        textureSize.x = CHARACTER_COL_SIZE;
        textureSize.y = CHARACTER_ROW_SIZE;
        
        glColor4ub(0xFF, 0xFF, 0xFF, 0xFF);

        GameUtil::Utilities::drawSprite(vertTR, vertBL, textureCoord, textureSize);
    }
}

//-----------------------------------------------------------------------------------------

void Character::shutdown()
{
}

//-----------------------------------------------------------------------------------------

void Character::handleMovement(DWORD milliseconds)
{
    Coord2D prevTilePos = mPrevTile->getPosition();
    float distFromPrev = GameUtil::Utilities::squareDistance(mPosition.x, prevTilePos.x, mPosition.y, prevTilePos.y);
    float distBetween = mPrevTile->getSquareDistToTile(mNextTile);

    // get the percent distance we've travelled from previous to next
    float percentTravelled = distFromPrev / distBetween;
    if(percentTravelled >= 0.99f)
    {
        // we are at (or somehow passed) the next tile, snap position
        Coord2D nextTilePos = mNextTile->getPosition();
        mPosition.x = nextTilePos.x;
        mPosition.y = nextTilePos.y;

        // special case for when we're at a teleporter
        //      snap to the teleporter's sibling, then set a boolean flag
        //          the boolean flag makes it so we don't bounce back and forth after
        //          the first teleport and instead move to the next non-teleporter
        //          so we can transition to AT_TILE
        if(mCurrTile->isTeleporter())
        {
            if(!mDidTeleport)
            {
                // snap position to the other teleporter
                mCurrTile = mCurrTile->getNeighbor(mFacingDirection);
                mPosition.x = mCurrTile->getPosition().x;
                mPosition.y = mCurrTile->getPosition().y;

                // target the next non-teleporter tile for movement
                mPrevTile = mCurrTile;
                mNextTile = mCurrTile->getNeighbor(mFacingDirection);

                mDidTeleport = true;
            }
        }
        else
        {
            mDidTeleport = false;
            mMovementState = MovementState::AT_TILE;
        }
    }
    else
    {
        // we are halfway there, migrate the current tile
        if(percentTravelled >= 0.5f)
        {
            mCurrTile = mNextTile;
        }

        // handle movement in our current direction
        switch(mFacingDirection)
        {
            case GameUtil::BoardDirection::UP:
                mPosition.y += mSpeed * milliseconds;
                break;

            case GameUtil::BoardDirection::LEFT:
                mPosition.x -= mSpeed * milliseconds;
                break;

            case GameUtil::BoardDirection::DOWN:
                mPosition.y -= mSpeed * milliseconds;
                break;

            case GameUtil::BoardDirection::RIGHT:
                mPosition.x += mSpeed * milliseconds;
                break;

            default:
                break;
        }
    }
}

//-----------------------------------------------------------------------------------------

void Character::unpause()
{
    mIsPaused = false;
}

//-----------------------------------------------------------------------------------------

void Character::pause()
{
    mIsPaused = true;
}

//-----------------------------------------------------------------------------------------

void Character::makeVisible()
{
    mIsVisible = true;
}

//-----------------------------------------------------------------------------------------

void Character::makeInvisible()
{
    mIsVisible = false;
}

//-----------------------------------------------------------------------------------------

const bool8_t Character::isAlive() const
{
    return mIsAlive;
}

//-----------------------------------------------------------------------------------------

void Character::setLiving(bool alive)
{
    mIsAlive = alive;
}

//-----------------------------------------------------------------------------------------

const GameUtil::CharacterID Character::getCharacterID() const
{
    return mCharacterID;
}

//-----------------------------------------------------------------------------------------

const Tile* Character::getCurrentTile() const
{
    return mCurrTile;
}

//-----------------------------------------------------------------------------------------

const GameUtil::BoardDirection Character::getFacingDirection() const
{
    return mFacingDirection;
}

//-----------------------------------------------------------------------------------------

const Character::MovementState Character::getMovementState() const
{
    return mMovementState;
}
