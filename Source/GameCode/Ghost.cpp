
#define GHOST_CPP

#include <stdio.h>
#include <windows.h>											// Header File For Windows
#include <gl\gl.h>											    // Header File For The OpenGL32 Library
#include <gl\glu.h>												// Header File For The GLu32 Library
#include <assert.h>
#include "..\\Framework\\baseTypes.h"
#include "..\\Framework\\random.h"

#include <map>
#include "Utilities.h"
#include "Tile.h"
#include "Character.h"
#include "Ghost.h"

#include "Pacman.h"
#include "GhostRed.h"
#include "GhostPink.h"
#include "GhostBlue.h"
#include "GhostOrange.h"
#include "BoardManager.h"
#include "CharacterManager.h"

#include "StatsManager.h"


Ghost::Ghost(const Tile* spawnTile, const GameUtil::CharacterID id) :
    Character(spawnTile, id)
{
}

//-----------------------------------------------------------------------------------------

void Ghost::reset()
{
    Character::reset();

    // reset booleans
    mIsInGhostHome = false;
    mIsScared = false;
    mIsFlaggedForReverse = false;

    // reset targeting pahse
    mCurrentPhase = GhostTargetingState::SCATTER;
    mTargetState = mCurrentPhase;

    // save stats from stats manager for the new level so we don't spam request them later
    const StatsManager* smInstance = StatsManager::ConstInstace();
    mGhostSpeed = smInstance->getGhostSpeed();
    mGhostSpeedScared = smInstance->getGhostSpeedScared();
    mGhostSpeedTunnel = smInstance->getGhostSpeedTunnel();

    // reset timers
    mScaredTimer = 0;
    mScaredDuration = smInstance->getGhostScaredTime();
    mFlashThreshold = smInstance->getGhostFlashTime();

    
}

//-----------------------------------------------------------------------------------------

void Ghost::resetFromDeath()
{
    // KEEP TARGETING PHASE AND TIMERS THE SAME

    // resetting from pacman death
    Character::resetFromDeath();

    mIsInGhostHome = false;
    mIsScared = false;
    mIsFlaggedForReverse = false;
}

//-----------------------------------------------------------------------------------------

void Ghost::updateTimers(DWORD milliseconds)
{
    if(!mIsPaused)
    {
        // update scared timer
        if(mIsScared)
        {
            if(mScaredTimer > 0)
            {
                mScaredTimer -= milliseconds;
                if(mAnimChunkID != GameUtil::AnimChunkID::ANIM_GHOST_FLASHING && mScaredTimer < mFlashThreshold)
                {
                    flash();
                }
            }
            else
            {
                unscare();
            }
        }
    }
}

//-----------------------------------------------------------------------------------------

void Ghost::updateMovement(DWORD milliseconds)
{
    if(!mIsPaused)
    {
        chooseTargetTile();
        Character::updateMovement(milliseconds);
    }
}

//-----------------------------------------------------------------------------------------

void Ghost::updateAnimation(DWORD milliseconds)
{
    // update animation frame
    Character::updateAnimation(milliseconds);

    // ghost can use its targeting state to set its animation chunk ID
    //      the animation chunk is the region of the spritesheet from which openGL will draw the ghost's quad
    switch(mTargetState)
    {
        // if chase or scatter, we display the base ghost
        case GhostTargetingState::CHASE: case GhostTargetingState::SCATTER:
            mAnimChunkID = static_cast<GameUtil::AnimChunkID>(mCharacterID);
            break;

        // if dead, we have to check if we're also in the ghost house (base ghost if so, else dead ghost)
        case GhostTargetingState::DEAD:
            if(mIsInGhostHome) mAnimChunkID = static_cast<GameUtil::AnimChunkID>(mCharacterID);
            else mAnimChunkID = GameUtil::AnimChunkID::ANIM_GHOST_DEAD;
            break;

        // default cases would be SCARED and FLASHING animation, which are set elsewhere
        default:
            break;
    }
}

//-----------------------------------------------------------------------------------------

void Ghost::updateSpeed()
{
    // speed is affected by ghost targeting mode
    switch(mTargetState)
    {
        // if scared...
        case GhostTargetingState::SCARED:
            mSpeed = mBaseSpeed * mGhostSpeedScared;
            break;

            // chase or scatter has to take into account the tile type
        case GhostTargetingState::CHASE: case GhostTargetingState::SCATTER:
            if(mCurrTile->isTunnel())
            {
                mSpeed = mBaseSpeed * mGhostSpeedTunnel;
            }
            else
            {
                mSpeed = mBaseSpeed * mGhostSpeed;
            }
            break;

            // dead ghosts take into account if they're in the ghost hosue
        case GhostTargetingState::DEAD:
            if(mIsInGhostHome)
            {
                mSpeed = mBaseSpeed * mGhostSpeed;
            }
            else
            {
                mSpeed = mBaseSpeed * 1.5f;
            }
            break;

        default:
            break;
    }
}

//-----------------------------------------------------------------------------------------

void Ghost::chooseTargetTile()
{
    switch(mTargetState)
    {
        // chase behavior, run ghost's specific targeting method
        case CHASE:
            targetPacman();
            break;

        // scatter behavior, target ghost's home corner
        case SCATTER:
            mTargetTile = BoardManager::ConstInstace()->getGhostHomeTile(mCharacterID);
            break;

        // dead, target ghost home
        case DEAD:
            mTargetTile = BoardManager::ConstInstace()->getGhostHomeTile(GameUtil::CharacterID::PACMAN);
            break;

        // scared or flashing, don't update target (has no bearing on navigation anyways)
        default:
            break;
    }
}

//-----------------------------------------------------------------------------------------

void Ghost::chooseNextTile()
{
    if(mIsFlaggedForReverse)
    {
        reverseDirection();
    }
    else
    {
        // how the ghost chooses its next tile depends on the current tile's type
        switch(mCurrTile->getTraverseType())
        {
            case Tile::TileTraverseType::TRAVERSABLE:
            case Tile::TileTraverseType::GHOST_TRAVERSABLE:
            case Tile::TileTraverseType::TUNNEL:
                handleBaseTraversable();
                break;

            case Tile::TileTraverseType::INTERSECTION:
                handleIntersection();
                break;

            case Tile::TileTraverseType::NO_UP_INTERSECTION:
                handleNoUpIntersection();
                break;

            case Tile::TileTraverseType::GHOST_HOME_ENTER:
                handleGhostHomeEnter();
                break;

            case Tile::TileTraverseType::GHOST_HOME_EXIT:
                handleGhostHomeExit();
                break;

            default:
                break;
        }
    }
}

//-----------------------------------------------------------------------------------------

void Ghost::flagForDirectionReverse()
{
    // can only reverse direction if outside ghost home
    if(!mIsInGhostHome && mIsAlive)
    {
        mIsFlaggedForReverse = true;
    }
}

//-----------------------------------------------------------------------------------------

void Ghost::setChasing()
{
    mCurrentPhase = GhostTargetingState::CHASE;
    if(mIsAlive && !mIsScared)
    {
        mTargetState = mCurrentPhase;
    }
}

//-----------------------------------------------------------------------------------------

void Ghost::setScatter()
{
    mCurrentPhase = GhostTargetingState::SCATTER;
    if(mIsAlive && !mIsScared)
    {
        mTargetState = mCurrentPhase;
    }
}

//-----------------------------------------------------------------------------------------

void Ghost::scare()
{
    mIsScared = true;
    mScaredTimer = mScaredDuration;
    mTargetState = GhostTargetingState::SCARED;
    mAnimChunkID = GameUtil::AnimChunkID::ANIM_GHOST_SCARED;

    mAnimFrame = 0;

    flagForDirectionReverse();
}

//-----------------------------------------------------------------------------------------

void Ghost::flash()
{
    if(mIsScared)
    {
        mAnimFrame = 0;
        mAnimChunkID = GameUtil::AnimChunkID::ANIM_GHOST_FLASHING;
    }
}

//-----------------------------------------------------------------------------------------

void Ghost::unscare()
{
    if(mIsScared)
    {
        // unscare ghost, enter current phase
        mIsScared = false;
        mScaredTimer = 0;
        mTargetState = mCurrentPhase;
        mAnimFrame = 0;
    }
}

//-----------------------------------------------------------------------------------------

void Ghost::setLiving(bool8_t isAlive)
{
    if(isAlive)
    {
        spawn();
    }
    else
    {
        kill();
    }
}

//-----------------------------------------------------------------------------------------

void Ghost::spawn()
{
    // can only spawn a ghost that is already dead and inside ghost home
    if(mIsInGhostHome)
    {
        mIsAlive = true;
        if(!mIsScared)
        {
            mTargetState = mCurrentPhase;
        }
    }
}

//-----------------------------------------------------------------------------------------

void Ghost::kill()
{
    // can only kill a ghost that is alive and outside ghost home
    if(!mIsInGhostHome)
    {
        // going back to ghost house
        unscare();
        mIsAlive = false;
        mTargetState = GhostTargetingState::DEAD;
    }
}

//-----------------------------------------------------------------------------------------

const Ghost::GhostTargetingState Ghost::getTargetingState() const
{
    return mTargetState;
}

//-----------------------------------------------------------------------------------------

const bool8_t Ghost::isGhostScared() const
{
    return mTargetState == GhostTargetingState::SCARED;
}

//-----------------------------------------------------------------------------------------

const bool8_t Ghost::isInGhostHome() const
{
    return mIsInGhostHome;
}





//-----------------------------------------------------------------------------------------

void Ghost::reverseDirection()
{
    mIsFlaggedForReverse = false;

    // reverse facing direction
    switch(mFacingDirection)
    {
        case GameUtil::BoardDirection::UP:
            mFacingDirection = GameUtil::BoardDirection::DOWN;
            break;
        case GameUtil::BoardDirection::LEFT:
            mFacingDirection = GameUtil::BoardDirection::RIGHT;
            break;
        case GameUtil::BoardDirection::DOWN:
            mFacingDirection = GameUtil::BoardDirection::UP;
            break;
        case GameUtil::BoardDirection::RIGHT:
            mFacingDirection = GameUtil::BoardDirection::LEFT;
            break;
    }

    nextTileChosen(mPrevTile, mFacingDirection);
}

//-----------------------------------------------------------------------------------------

void Ghost::handleBaseTraversable()
{
    const Tile* neighbor = nullptr;
    GameUtil::BoardDirection direction = GameUtil::BoardDirection::INVAL;

    // iterate through all directions, and pick the first traversable tile we find that is not our previous tile
    for(uint32_t directionInt = GameUtil::BoardDirection::UP; directionInt != GameUtil::BoardDirection::INVAL; ++directionInt)
    {
        direction = static_cast<GameUtil::BoardDirection>(directionInt);
        neighbor = mCurrTile->getNeighbor(direction);

        if(neighbor != mPrevTile && neighbor->isTraversable())
        {
            nextTileChosen(neighbor, direction);
            break;
        }
    }
}

//-----------------------------------------------------------------------------------------

void Ghost::handleIntersection()
{
    switch(mTargetState)
    {
        case GhostTargetingState::SCARED:
            chooseRandomNeighbor();
            break;

        default:
            chooseNeighborClosestToTarget(GameUtil::BoardDirection::INVAL);
            break;
    }
}

//-----------------------------------------------------------------------------------------

void Ghost::handleNoUpIntersection()
{
    switch(mTargetState)
    {
        // ghosts ignore this rule when in scared
        case GhostTargetingState::SCARED:
            chooseRandomNeighbor();
            break;

        // also while dead
        case GhostTargetingState::DEAD:
            chooseNeighborClosestToTarget(GameUtil::BoardDirection::INVAL);
            break;
            
        // otherwise, ignore up
        default:
            chooseNeighborClosestToTarget(GameUtil::BoardDirection::UP);
            break;
    }
}

//-----------------------------------------------------------------------------------------

void Ghost::handleGhostHomeEnter()
{
    mIsInGhostHome = false;     // just outside ghost home

    if(mIsAlive)
    {
        // treat as normal intersection, ignore down so we don't accidentally reenter ghost home
        chooseNeighborClosestToTarget(GameUtil::BoardDirection::DOWN);
    }
    else
    {
        // else, turn down into the ghost home
        nextTileChosen(mCurrTile->getNeighbor(GameUtil::BoardDirection::DOWN), GameUtil::BoardDirection::DOWN);
    }
}

//-----------------------------------------------------------------------------------------

void Ghost::handleGhostHomeExit()
{
    mIsInGhostHome = true;      // just inside ghost home

    if(mIsAlive)
    {
        // if alive, turn up out of the ghost home
        nextTileChosen(mCurrTile->getNeighbor(GameUtil::BoardDirection::UP), GameUtil::BoardDirection::UP);
    }
    else
    {
        // if dead, continue left
        nextTileChosen(mCurrTile->getNeighbor(GameUtil::BoardDirection::LEFT), GameUtil::BoardDirection::LEFT);
    }
}

//-----------------------------------------------------------------------------------------

void Ghost::chooseNeighborClosestToTarget(GameUtil::BoardDirection skipDirection)
{
    float dist = 0.0f;
    float minDist = FLT_MAX;
    GameUtil::BoardDirection minDir = GameUtil::BoardDirection::INVAL;

    const Tile* neighbor = nullptr;
    GameUtil::BoardDirection direction = GameUtil::BoardDirection::INVAL;

    // iterate through all directions, skip given direction
    //      select direction that would put ghost closest to its target tile
    for(uint32_t directionInt = GameUtil::BoardDirection::UP; directionInt != GameUtil::BoardDirection::INVAL; ++directionInt)
    {
        direction = static_cast<GameUtil::BoardDirection>(directionInt);
        if(direction == skipDirection) continue;

        // if the current neighbor is not the previous tile and is traversable...
        neighbor = mCurrTile->getNeighbor(direction);
        if(neighbor != mPrevTile && neighbor->isGhostTraversable())
        {
            // check its distance, save direction of min values
            //      since min is overwritten only on '<', ties are given priority via the order of the map
            //      UP, LEFT, DOWN (RIGHT is never chosen in a tie because everything else is higher priority)
            dist = neighbor->getSquareDistToTile(mTargetTile);
            if(dist < minDist)
            {
                minDist = dist;
                minDir = direction;
            }
        }
    }

    // we now have the direction that would put us closest to the target
    nextTileChosen(mCurrTile->getNeighbor(minDir), minDir);
}

//-----------------------------------------------------------------------------------------

void Ghost::chooseRandomNeighbor()
{
    // choose a random direction
    GameUtil::BoardDirection randomDir = static_cast<GameUtil::BoardDirection>(getRangedRandom(GameUtil::BoardDirection::UP, GameUtil::BoardDirection::INVAL));
    const Tile* randomTile = mCurrTile->getNeighbor(randomDir);

    // if the direction is not valid for movement, cycle through each direction and choose the first
    // traversable direction that is not our previous direction
    if(!randomTile->isTraversable() || randomTile == mPrevTile)
    {
        for(uint32_t directionInt = GameUtil::BoardDirection::UP; directionInt != GameUtil::BoardDirection::INVAL; ++directionInt)
        {
            randomDir = static_cast<GameUtil::BoardDirection>(directionInt);
            randomTile = mCurrTile->getNeighbor(randomDir);

            if(randomTile != mPrevTile && randomTile->isTraversable())
            {
                break;
            }
        }
    }

    nextTileChosen(randomTile, randomDir);
}

//-----------------------------------------------------------------------------------------

void Ghost::nextTileChosen(const Tile* next, GameUtil::BoardDirection direction)
{
    mNextTile = next;
    mPrevTile = mCurrTile;

    mFacingDirection = direction;

    mMovementState = MovementState::MOVING;
}
