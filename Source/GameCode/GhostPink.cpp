
#define GHOST_PINK_CPP

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
#include "GhostPink.h"

#include "CharacterManager.h"
#include "BoardManager.h"

GhostPink::GhostPink(const Tile* spawnTile, const GameUtil::CharacterID id) :
    Ghost(spawnTile, id)
{
    // init inside ghost home
    mIsInGhostHome = true;
    mIsAlive = false;

    // PINKY
    // SPEEDY
}

//-----------------------------------------------------------------------------------------

void GhostPink::reset()
{
    Ghost::reset();

    mIsInGhostHome = true;
    mIsAlive = false;
}

//-----------------------------------------------------------------------------------------

void GhostPink::resetFromDeath()
{
    Ghost::resetFromDeath();

    mIsInGhostHome = true;
    mIsAlive = false;
}

//-----------------------------------------------------------------------------------------

void GhostPink::targetPacman()
{
    const CharacterManager* characterManagerInstance = CharacterManager::ConstInstance();
    const BoardManager* boardManagerInstance = BoardManager::ConstInstace();

    GameUtil::BoardDirection pacmanFacing = characterManagerInstance->getCharacterFacingDirection(GameUtil::CharacterID::PACMAN);
    const Tile* pacmanTile = characterManagerInstance->getCharacterCurrentTile(GameUtil::CharacterID::PACMAN);

    // targets 4 spaces ahead of pacman
    if(pacmanFacing == GameUtil::BoardDirection::UP)
    {
        // recreating overflow bug from original game causing Pinky to target 4 above and 4 left of pacman
        mTargetTile = boardManagerInstance->lookNumTilesAhead(pacmanTile, 4, GameUtil::BoardDirection::UP);
        mTargetTile = boardManagerInstance->lookNumTilesAhead(mTargetTile, 4, GameUtil::BoardDirection::LEFT);
    }
    else
    {
        mTargetTile = boardManagerInstance->lookNumTilesAhead(pacmanTile, 4, pacmanFacing);
    }
}
