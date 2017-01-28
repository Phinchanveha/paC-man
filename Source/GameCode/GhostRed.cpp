
#define GHOST_RED_CPP

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
#include "Ghost.h"
#include "GhostRed.h"

#include "CharacterManager.h"

#include "StatsManager.h"

GhostRed::GhostRed(const Tile* spawnTile, const GameUtil::CharacterID id) :
    Ghost(spawnTile, id)
{
    // BLINKY
    // SHADOW
}

//-----------------------------------------------------------------------------------------

void GhostRed::reset()
{
    // reset for the new level, then reset cruise elroy mode
    Ghost::reset();
    mCruiseElroyMode = GameUtil::CruiseElroyMode::NONE;

    // save cruise elroy speed values from the stats manager so we don't spam request them
    const StatsManager* smInstance = StatsManager::ConstInstace();
    mCruiseElroySpeed_1 = smInstance->getCruiseElroySpeed_1();
    mCruiseElroySpeed_2 = smInstance->getCruiseElroySpeed_2();
}

//-----------------------------------------------------------------------------------------

void GhostRed::resetFromDeath()
{
    // reset from pacman death, then reset cruise elroy mode
    Ghost::resetFromDeath();
    mCruiseElroyMode = GameUtil::CruiseElroyMode::NONE;
}

//-----------------------------------------------------------------------------------------

void GhostRed::targetPacman()
{
    // targets pacman directly
    mTargetTile = CharacterManager::ConstInstance()->getCharacterCurrentTile(GameUtil::CharacterID::PACMAN);
}

//-----------------------------------------------------------------------------------------

void GhostRed::updateSpeed()
{
    // speed is affected by ghost targeting mode
    switch(mTargetState)
    {
        // if scared...
        case GhostTargetingState::SCARED:
            mSpeed = mBaseSpeed * mGhostSpeedScared;
            break;

        // chase or scatter has to take into account the tile type (except in cruise elroy mode)
        case GhostTargetingState::CHASE: case GhostTargetingState::SCATTER:
            switch(mCruiseElroyMode)
            {
                case GameUtil::CruiseElroyMode::NONE:
                    if(mCurrTile->isTunnel())
                    {
                        mSpeed = mBaseSpeed * mGhostSpeedTunnel;
                    }
                    else
                    {
                        mSpeed = mBaseSpeed * mGhostSpeed;
                    }
                    break;
                case GameUtil::CruiseElroyMode::CRUISE_1:
                    mSpeed = mBaseSpeed * mCruiseElroySpeed_1;
                    break;
                case GameUtil::CruiseElroyMode::CRUISE_2:
                    mSpeed = mBaseSpeed * mCruiseElroySpeed_2;
                    break;
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

void GhostRed::setScatter()
{
    // can only scatter when not in a cruise elroy mode
    if(mCruiseElroyMode == GameUtil::CruiseElroyMode::NONE)
    {
        Ghost::setScatter();
    }
}

//-----------------------------------------------------------------------------------------

void GhostRed::activateCruiseElroy()
{
    GameUtil::CruiseElroyMode nextPhase = (GameUtil::CruiseElroyMode)(mCruiseElroyMode + 1);
    if(nextPhase != GameUtil::CruiseElroyMode::CRUISE_INVAL)
    {
        // can only experience a cruise elroy state change when all ghosts are spawned
        if(CharacterManager::ConstInstance()->areAllGhostsSpawned())
        {
            mCruiseElroyMode = nextPhase;
        }
    }
}

//-----------------------------------------------------------------------------------------

const  GameUtil::CruiseElroyMode  GhostRed::getCruiseElroyPhase() const
{
    return mCruiseElroyMode;
}
