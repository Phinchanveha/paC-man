
#define PACMAN_CPP

#include <stdio.h>
#include <windows.h>											// Header File For Windows
#include <gl\gl.h>											    // Header File For The OpenGL32 Library
#include <gl\glu.h>												// Header File For The GLu32 Library
#include <assert.h>
#include "..\\Framework\\baseTypes.h"
#include "..\\Framework\\InputManager.h"
#include "..\\Framework\\openGLFramework.h"

#include <map>
#include "Utilities.h"
#include "Tile.h"
#include "Character.h"
#include "Pacman.h"

#include "Ghost.h"
#include "GhostRed.h"
#include "GhostPink.h"
#include "GhostBlue.h"
#include "GhostOrange.h"
#include "BoardManager.h"
#include "CharacterManager.h"

#include "StatsManager.h"

#define DEATH_FRAME_DELTA_TIME  250
#define PACMAN_FRAME_DELTA_TIME 50

Pacman::Pacman(const Tile* spawnTile, const GameUtil::CharacterID id) :
    Character(spawnTile, id)
{
}

//-----------------------------------------------------------------------------------------

void Pacman::reset()
{
    // reset stuff on the character class
    Character::reset();

    mFacingDirection = GameUtil::BoardDirection::UP;

    mFrameDeltaTime = PACMAN_FRAME_DELTA_TIME;

    // save pacman's speed values for the new level so we don't spam request them from the stats manager
    const StatsManager* smInstance = StatsManager::ConstInstace();
    mPacmanSpeed = smInstance->getPacmanSpeed();
    mPacmanSpeedDots = smInstance->getPacmanSpeedDots();
    mPacmanSpeedScared = smInstance->getPacmanScaredSpeed();
    mPacmanSpeedScaredDots = smInstance->getPacmanScaredSpeedDots();
}

//-----------------------------------------------------------------------------------------

void Pacman::resetFromDeath()
{
    Character::resetFromDeath();

    mFacingDirection = GameUtil::BoardDirection::UP;

    mFrameDeltaTime = PACMAN_FRAME_DELTA_TIME;
}

//-----------------------------------------------------------------------------------------

void Pacman::updateMovement(DWORD milliseconds)
{
    if(mIsAlive)
    {
        Character::updateMovement(milliseconds);
        updateUserInput();
    }
}

//-----------------------------------------------------------------------------------------

void Pacman::updateAnimation(DWORD milliseconds)
{
    if(mMovementState == MovementState::MOVING || !mIsAlive)
    {
        Character::updateAnimation(milliseconds);
    }
}

//-----------------------------------------------------------------------------------------

void Pacman::updateSpeed()
{
    // pacman uses the ghosts' scared state and whether his current tile has a pellet on it
    if(CharacterManager::ConstInstance()->areGhostsScared())
    {
        // speeds if ghosts are scared
        if(mCurrTile->isPelletPresent())
        {
            mSpeed = mBaseSpeed * mPacmanSpeedScaredDots;
        }
        else
        {
            mSpeed = mBaseSpeed * mPacmanSpeedScared;
        }
    }
    else
    {
        // speeds if ghosts are not scared
        if(mCurrTile->isPelletPresent())
        {
            mSpeed = mBaseSpeed * mPacmanSpeedDots;
        }
        else
        {
            mSpeed = mBaseSpeed * mPacmanSpeed;
        }
    }
}

//-----------------------------------------------------------------------------------------

void Pacman::playDeathAnimation()
{
    // unpause pacman so the animation will actually play
    mIsPaused = false;

    // reset the animation frames, and make the animation play slightly slower than normal
    mAnimFrame = 0;
    mTimeOnCurrFrame = 0;
    mFrameDeltaTime = DEATH_FRAME_DELTA_TIME;
    mAnimChunkID = GameUtil::AnimChunkID::ANIM_PACMAN_DEAD;
    mFacingDirection = GameUtil::BoardDirection::UP;
}

//-----------------------------------------------------------------------------------------

void Pacman::updateUserInput()
{
    if(mIsAlive && !mIsPaused)
    {
        // pacman automatically selects his current direction of movement
        GameUtil::BoardDirection direction = mFacingDirection;

        // select direction using arrow key input
        if(g_keys->keyDown[VK_UP])
        {
            direction = GameUtil::BoardDirection::UP;
        }
        else if(g_keys->keyDown[VK_LEFT])
        {
            direction = GameUtil::BoardDirection::LEFT;
        }
        else if(g_keys->keyDown[VK_DOWN])
        {
            direction = GameUtil::BoardDirection::DOWN;
        }
        else if(g_keys->keyDown[VK_RIGHT])
        {
            direction = GameUtil::BoardDirection::RIGHT;
        }

        // user input may change pacman's facing direction at any time,
        // so we must consider pacman's next movement target whenever we receive new input
        if(direction != mFacingDirection)
        {
            mFacingDirection = direction;

            const Tile* tempTile = mCurrTile->getNeighbor(direction);
            if(tempTile != nullptr)
            {
                // if target tile is traversable
                // we have to consider pacman's distance to the current tile center
                if(tempTile->isTraversable())
                {
                    Coord2D currTilePos = mCurrTile->getPosition();
                    float distToCurr = GameUtil::Utilities::squareDistance(mPosition.x, currTilePos.x, mPosition.y, currTilePos.y);
                    if(mMovementState == MovementState::MOVING)
                    {
                        // if pacman is close enough to the current tile's center, he snaps to the tile's position and moves in that direction
                        //      if the player's timing is right, pacman can take corners faster than the ghosts in this way
                        mPosition.x = currTilePos.x;
                        mPosition.y = currTilePos.y;
                    }
                }
            }

            mMovementState = MovementState::AT_TILE;
        }
    }
}

//-----------------------------------------------------------------------------------------

void Pacman::chooseNextTile()
{
    if(mIsAlive)
    {
        // automatically select the next movement target in the current direction
        const Tile* tempTile = mCurrTile->getNeighbor(mFacingDirection);
        if(tempTile != nullptr)
        {
            mNextTile = tempTile;
            mPrevTile = mCurrTile;

            // if pacman's next movement target is traversable,
            // we transition states and begin moving towards it
            if(mNextTile->isTraversable())
            {
                mMovementState = MovementState::MOVING;
            }
        }
    }
}
