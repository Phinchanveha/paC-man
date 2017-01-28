
#define GHOST_BLUE_CPP

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
#include "GhostBlue.h"

#include "CharacterManager.h"
#include "BoardManager.h"

GhostBlue::GhostBlue(const Tile* spawnTile, const GameUtil::CharacterID id) :
    Ghost(spawnTile, id)
{
    // INKY
    // BASHFUL
}

//-----------------------------------------------------------------------------------------

void GhostBlue::reset()
{
    Ghost::reset();

    mIsInGhostHome = true;
    mIsAlive = false;
}

//-----------------------------------------------------------------------------------------

void GhostBlue::resetFromDeath()
{
    Ghost::resetFromDeath();

    mIsInGhostHome = true;
    mIsAlive = false;
}

//-----------------------------------------------------------------------------------------

void GhostBlue::targetPacman()
{
    const CharacterManager* characterManagerInstance = CharacterManager::ConstInstance();
    const BoardManager* boardManagerInstance = BoardManager::ConstInstace();

    GameUtil::BoardDirection pacmanFacing = characterManagerInstance->getCharacterFacingDirection(GameUtil::CharacterID::PACMAN);
    const Tile* pacmanTile = characterManagerInstance->getCharacterCurrentTile(GameUtil::CharacterID::PACMAN);
    
    // get tile 2 spaces ahead of pacman
    if(pacmanFacing == GameUtil::BoardDirection::UP)
    {
        // recreating overflow bug from original game causing Inky to use 2 above and 2 left of pacman
        mTargetTile = boardManagerInstance->lookNumTilesAhead(pacmanTile, 2, GameUtil::BoardDirection::UP);
        mTargetTile = boardManagerInstance->lookNumTilesAhead(mTargetTile, 2, GameUtil::BoardDirection::LEFT);
    }
    else
    {
        mTargetTile = boardManagerInstance->lookNumTilesAhead(pacmanTile, 2, pacmanFacing);
    }

    // get the x,y number of tiles between Blinky and 2 spaces ahead of pacman
    // the target is offset by that much from 2 spaces ahead of pacman
    const Tile* blinkyTile = characterManagerInstance->getCharacterCurrentTile(GameUtil::CharacterID::GHOST_RED);
    Coord2D tileBlinkyIndex = blinkyTile->getBoardIndex();
    Coord2D tileAheadIndex = mTargetTile->getBoardIndex();

    int stepsX = tileAheadIndex.x - tileBlinkyIndex.x;
    if(stepsX >= 0)
    {
        mTargetTile = boardManagerInstance->lookNumTilesAhead(mTargetTile, stepsX, GameUtil::BoardDirection::RIGHT);
    }
    else
    {
        mTargetTile = boardManagerInstance->lookNumTilesAhead(mTargetTile, -stepsX, GameUtil::BoardDirection::LEFT);
    }

    int stepsY = tileAheadIndex.y - tileBlinkyIndex.y;
    if(stepsY >= 0)
    {
        mTargetTile = boardManagerInstance->lookNumTilesAhead(mTargetTile, stepsY, GameUtil::BoardDirection::UP);
    }
    else
    {
        mTargetTile = boardManagerInstance->lookNumTilesAhead(mTargetTile, -stepsY, GameUtil::BoardDirection::DOWN);
    }
}
